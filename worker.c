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

int worker(int task_id,int totallinker)
{
    int     sendfd;
    struct  msghdr msg = {0};
    struct  cmsghdr *cmsg;
    char    buf[CMSG_SPACE(sizeof(int))];  /* ancillary data buffer for getting fd*/
    struct  iovec iov[1];
    int     rc = 0;
    struct  sockaddr_un addr;
    char    path[500];
    int     unix_server_fd; 
    int     current_linker;

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
    /* Initialize the payload: */
    snprintf(path,500,"/home/zdx/%d.sock",task_id);

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
        int writefd = *(int *)CMSG_DATA(cmsg);
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
