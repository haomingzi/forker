#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFSIZE (64*1024)

struct msghead{
   int  id;
   int  size;
   char payload[];
};

struct request{
   int type;
   int linkcount;
   int taskid;
};

int main(int argc,char *argv){

    int fd = socket(AF_INET,SOCK_STREAM,0);
    FILE *pf=NULL;

    struct sockaddr_in    servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if(connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
         printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
         exit(0);
     }

    char* buf=malloc(BUFSIZE);
    struct msghead* head=(struct msghead*)buf;
    struct request* req=(struct request *)head->payload;
    head->id=htonl(101);
    head->size=htonl(sizeof(struct request));
    req->linkcount=htonl(2);
    req->type=htonl(1);
    req->taskid=htonl(103);
    pf=fopen("temp.txt","w");
/*
    int len = 0;
    int loop  = 0;
        len=sprintf(&buf[4],"create_dispatch file_list=/home/gbase/data format=3 node_count=6 auto_fill_column=1 using_direct_io=0 hash_parallel=4 delimiter=\x2C scn=1000 agent_list=10.10.120.111:5050");
        printf("len is %d\n",len);
        buf[0] = len;
*/
    write(fd,buf,sizeof(struct msghead)+sizeof(struct request));
    int i=0;
    int len;
    int total=0;

    while(1){
        len=read(fd,buf,BUFSIZE);
        if(len<=0){
            break;
        }
        fwrite(buf,len,1,pf);
        total+=len;
    }
    printf("total is %d\n",total);

    fclose(pf);
    close(fd);
    return 0;
}
