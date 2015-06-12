#ifndef GDEF_H
#define GDEF_H
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

void list_init(struct list_head* head);
void list_insert(struct list_head* head,struct list_head* fd);
struct list_head* list_iter(struct list_head* head);
void list_release(struct list_head* head);
void delete_fd(struct fd_list *fdlist,int fd);

#endif

