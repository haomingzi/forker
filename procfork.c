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

static int first = 1;
static int fd;
static int newfd;

int create_unix_server(int taskid){
    char path[500];
    int rc;
    int fd;
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

    return rc;
}

int fork_and_send(int fd_to_send,int taskid)
{
    int sendfd;
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
    iov[0].iov_base=&taskid;
    iov[0].iov_len=sizeof(taskid);
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    /* Initialize the payload: */

    if(first == 1){
        first = 0;
        rc=create_unix_server(taskid);
        if(rc < 0){
            return rc;
        }
        pid_t pid=fork();
        if(pid < 0){
            printf("fork faild\n");
            close(fd);
            exit(7);
        }else if(pid == 0){
            struct sockaddr_un addr;
            char path[500];
            int  unix_server_fd; 

            close(fd_to_send);
            close(fd);
            snprintf(path,500,"/home/zdx/%d.sock",taskid);

            addr.sun_family=AF_UNIX;
            strcpy(addr.sun_path,path);

            unix_server_fd=socket(AF_UNIX,SOCK_STREAM,0);
            if(unix_server_fd < 0){
                printf("child fd connect failed\n");
                exit(1);
            }

            rc = connect(unix_server_fd,(struct sockaddr*)&addr, sizeof(struct sockaddr_un));
            if(rc < 0){
                close(unix_server_fd);
                printf("connetc failed %s \n",strerror(errno));
                exit(10);
            }

            while(1){
                rc=recvmsg(unix_server_fd,&msg,0);
                if(rc < 0){
                    close(unix_server_fd);
                    printf("recv msg failed %s\n",strerror(errno));
                    exit(7);
                }
                fdptr = (int *)CMSG_DATA(cmsg);
                int writefd= *fdptr;
                rc =  write(writefd,"good\n",5);
                if(rc < 0){
                    close(unix_server_fd);
                    printf("write failed");
                    exit(8);
                }
                printf("write success\n");
                close(writefd);
            }
            close(unix_server_fd);
            exit(0);
        }

        socklen_t newsocklen;
        newfd = accept(fd,NULL,0);
        if(newfd == -1){
            printf("accept faild %s\n",strerror(errno));
            exit(5);
        }
        close(fd);
    }
    
    fdptr = (int *) CMSG_DATA(cmsg);
    *fdptr=fd_to_send;
    rc = sendmsg(newfd,&msg,0);
    if(rc == -1){
        printf("send msg failed");
        exit(6);
    }

    return 0;
}
