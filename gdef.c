#include <stdlib.h>
#include "gdef.h"


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

