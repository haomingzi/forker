#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "gdef.h"
#include "worker.h"

static workmap workers;
static int send_task_info(int sender_fd,struct request *req);

int create_unix_server(int taskid){
    char   path[500];
    int    rc = -1;
    int    fd;
    struct sockaddr_un addr;

    snprintf(path,500,"/tmp/%d.sock",taskid);

    addr.sun_family=AF_UNIX;
    strcpy(addr.sun_path,path);

    unlink(path);
    fd  = socket(AF_UNIX,SOCK_STREAM,0);
    if(fd == -1){
        printf("create unix socket error");
        return -1;
    }

    rc = bind(fd,(struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if(rc == -1){
        printf("bind failed %s\n",strerror(errno));
        return rc;
    }

    rc = listen(fd,5);
    if(rc == -1){
        printf("listen faild");
        return rc;
    }

    return fd;
}

int fork_and_send(int fd_to_send,struct request *req)
{
    /* Initialize the payload: */
    int        rc=-1;
    static int first = 1;
    int        unix_server_fd;
    if(NULL==workmap_find(&workers,req->taskid)){
        int  unixfd;
        work newwork;
        unix_server_fd=create_unix_server(req->taskid);
        if(unix_server_fd < 0){
            return -1;
        }
        pid_t pid=fork();
        if(pid < 0){
            printf("fork faild %d\n",errno);
            close(unix_server_fd);
            return -1;
        }else if(pid == 0){
            close_all_files();
            workmap_clear(&workers);
            worker(req->taskid,req->linkcount);
        }

        unixfd = accept(unix_server_fd,NULL,0);
        if(unixfd == -1){
            printf("accept faild %s\n",strerror(errno));
            exit(5);
        }

        newwork.commfd=unixfd;
        newwork.totallinker=req->linkcount;
        newwork.currentlinker=0;
        workmap_insert(&workers,req->taskid,newwork);
        close(unix_server_fd);
    }

    work *pw=workmap_find(&workers,req->taskid);
    work_ref_inc(pw);
    rc=send_task_info(pw->commfd,req);
    if(rc<0)
        return rc;

    if(work_finish(pw)){
        close(pw->commfd);
        workmap_delete(&workers,req->taskid);
    }

    return 0;
}

static int send_task_info(int sender_fd,struct request *req)
{
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(sizeof(int))];  /* ancillary data buffer */
    struct iovec iov[1];
    int rc = 0;

    msg.msg_control = buf;
    msg.msg_controllen = sizeof buf;
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    msg.msg_name=NULL;
    msg.msg_namelen=0;
    iov[0].iov_base=req;
    iov[0].iov_len=sizeof(struct request);
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    *(int *)CMSG_DATA(cmsg)=sender_fd;

    rc = sendmsg(sender_fd,&msg,0);
    if(rc == -1){
        printf("send msg failed");
        return -1;
    }

    return 0;
}
