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
#include "worker.h"

static int send_task_info(int sender_fd,int fd_to_send,int taskid);
static int unixfd;
int create_unix_server(int taskid){
    char   path[500];
    int    rc = -1;
    int    fd;
    struct sockaddr_un addr;

    snprintf(path,500,"/home/zdx/%d.sock",taskid);

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

int fork_and_send(int fd_to_send,int taskid)
{
    /* Initialize the payload: */
    int        rc=-1;
    static int first = 1;
    int        unix_server_fd;
    if(first == 1){
        first = 0;
        unix_server_fd=create_unix_server(taskid);
        if(unix_server_fd < 0){
            return -1;
        }
        pid_t pid=fork();
        if(pid < 0){
            printf("fork faild %d\n",errno);
            close(unix_server_fd);
            return -1;
        }else if(pid == 0){
            close(fd_to_send);
            close(unix_server_fd);
            worker(taskid);
        }

        unixfd = accept(unix_server_fd,NULL,0);
        if(unixfd == -1){
            printf("accept faild %s\n",strerror(errno));
            exit(5);
        }
        close(unix_server_fd);
    }

    rc=send_task_info(unixfd,fd_to_send,taskid);
    if(rc<0)
        return rc;
    return 0;
}

static int send_task_info(int sender_fd,int fd_to_send,int task_id)
{
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    int myfds; /* Contains the file descriptors to pass. */
    char buf[CMSG_SPACE(sizeof myfds)];  /* ancillary data buffer */
    int *fdptr;
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
    iov[0].iov_base=&task_id;
    iov[0].iov_len=sizeof(task_id);
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    *(int *) CMSG_DATA(cmsg) = fd_to_send;

    rc = sendmsg(sender_fd,&msg,0);
    if(rc == -1){
        printf("send msg failed");
        return -1;
    }

    return 0;
}
