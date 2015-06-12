#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "procfork.h"

struct msghead{
    int  id;
    int  size;
    char payload[];
};

struct request{
    int linkcount;
    int taskid;
};

struct list_head{
    struct list_head *next;
};

struct fd_list{
    struct list_head head;
    int    fd;
};

void list_init(struct list_head* head){
    head->next=NULL;
}

void list_insert(struct list_head* head,struct list_head* fd){
   fd->next=head->next;
   head->next=fd;
}

struct list_head* list_iter(struct list_head* head){
    return head->next;
}

void list_release(struct list_head* head){
    struct list_head *iter;
    for(iter=head->next;iter!=NULL;iter=head){
        head=iter->next;
        free(iter);
    }
}

void delete_fd(struct fd_list *fdlist,int fd){
   struct list_head *h=(struct list_head *)fdlist;
   struct list_head *iter;
   for(iter=fdlist->head.next;iter!=NULL;h=iter,iter=iter->next){
       if(((struct fd_list*)iter)->fd == fd){
           h->next=iter->next;
           free(iter);
       }
   }
}
    
struct fd_list fds;

int create_server(int port);
int accept_conn(int timeout);
int listenfd=-1;

int main(int argc,char *argv[])
{
    int rc=create_server(6666);
    if(rc!=0)
        return rc;

    accept_conn(5000);
    return 0;
}


int create_server(int port)
{
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd==-1){
        printf("create listen socket failed\n");
        return errno;
    }

    int reuse = 1;
    int rc=setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
    if(rc == -1){
        printf("setsockopt error happends! %s\n",strerror(errno));
        return 5;
    }

    struct sockaddr_in addr={
        .sin_family=AF_INET,
        .sin_port=htons(port)
    };

    addr.sin_addr.s_addr=htons(INADDR_ANY);

    rc=bind(listenfd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in));
    if(rc == -1){
        close(listenfd);
        printf("can't bind to the * port\n");
        return errno;
    }

    rc=listen(listenfd,10);
    if(rc == -1){
        close(listenfd);
        printf("can't listen to the fd %d\n",listenfd);
        return errno;
    }

    return 0;
}

int insert_fd(int fd){
    struct fd_list* entry;
    entry=(struct fd_list*)malloc(sizeof(struct fd_list));
    if(entry == NULL){
        printf("malloc entry failed\n");
        list_release((struct list_head*)&fds);
        return -1;
    }
    entry->fd=fd;
    list_insert((struct list_head*)&fds,(struct list_head*)entry);
    return 0;
}

int accept_conn(int timeout){

    char buffer[1024];
    struct pollfd pfd[100];
    int acceptfd=-1;
    int i=0;
    int rc = 0;

    list_init((struct list_head*)&fds);
    insert_fd(listenfd);

    while(1){
        struct list_head *iter=NULL;
        int    list_size = 0;
        for(iter=fds.head.next;iter!=NULL;iter=iter->next){
            pfd[list_size].fd=((struct fd_list *)iter)->fd;
            pfd[list_size].events=POLLIN;
            pfd[list_size].revents=0;
            list_size++;
        }

        rc=poll(pfd,list_size,timeout);
        if(rc==-1){
            printf("poll error happens %d ",errno);
            return -1;
        }else if(rc==0){
            printf("poll timeout happens second poll\n");
            continue;
        }
    
        for(i=0;i<list_size;i++){
            if(pfd[i].revents&POLLIN){
                if(pfd[i].fd == listenfd){
                    acceptfd=accept(listenfd,NULL,NULL);
                    if(acceptfd == -1){
                        printf("accept error %d\n",errno);
                        return -2;
                    }
                    fcntl(acceptfd,F_SETFL,O_NONBLOCK);
                    rc=insert_fd(acceptfd);
                    if(rc < 0){
                        printf("insert fd failed\n");
                        return rc;
                    }
                    printf("success accept a conn\n");
                }else{
                    int len=read(pfd[i].fd,buffer,1024);
                    if(len < 0){
                        printf("read error %d %s\n",pfd[i].fd,strerror(errno));
                        return -1;
                    }else if(len ==0){
                        printf("deleted a fd %d\n",pfd[i].fd);
                        delete_fd(&fds,pfd[i].fd);
                        close(pfd[i].fd);
                    }else{
                        printf("received a message len %d\n",len);
                        struct msghead *head=(struct msghead *)buffer;
                        if(ntohl(head->size) != sizeof(struct request)){
                            printf("received invalid len msg\n");
                        }else{
                            struct request *req=(struct request *)head->payload;
                            printf("received task id %d %d %d\n",ntohl(head->id),ntohl(req->linkcount),ntohl(req->taskid));
                            fork_and_send(pfd[i].fd,ntohl(head->id));
                        }
                        delete_fd(&fds,pfd[i].fd);
                        close(pfd[i].fd);
                    }
               }
            }else if(pfd[i].revents&POLLNVAL){
               printf("fd is not open\n");
            }else if(pfd[i].revents&POLLERR){
               printf("poll err happends\n");
            }else if(pfd[i].revents&POLLHUP){
               printf("poll hup happends\n");
            }
        }
    }
    return rc;
}

