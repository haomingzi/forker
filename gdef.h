#ifndef  GDEF_H
#define  GDEF_H
#include <set>
#include <map>

using namespace std;

struct msghead{
    int  id;
    int  size;
    char payload[];
};

struct request{
    int linkcount;
    int taskid;
};

typedef set<int> fdset;
typedef map<int,int> workmap;

struct list_head{
    struct list_head *next;
    void *payload;
};

struct fd_list{
    struct list_head head;
    int    fd;
};

#define list_entry(_set,fd)  \
    for(fdset::iterator iter=_set.begin();iter!=_set.end();iter++){ \
        fd = (*iter);

#define list_entry_end  }

void fdset_init(fdset *_set);
void fdset_insert(fdset *_set,int fd);
void fdset_delete(fdset *_set,int fd);
void fdset_clear(fdset *_set);
bool workmap_find(workmap *_map,int taskid);
void workmap_insert(workmap *_map,int taskid,int fd);
void workmap_delete(workmap *_map,int taskid);

#endif

