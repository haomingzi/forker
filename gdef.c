#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "gdef.h"

void fdset_init(fdset *_set){
    _set->clear();
}

void fdset_insert(fdset *_set,int fd){
    _set->insert(fd);
}

void fdset_delete(fdset *_set,int fd){
    _set->erase(fd);
}

void fdset_clear(fdset *_set){
    _set->clear();
}


work* workmap_find(workmap *_map,int taskid){
    return (_map->end() != _map->find(taskid))?&(*_map)[taskid]:NULL;
}
void workmap_insert(workmap *_map,int taskid,work worker){
   (*_map)[taskid]=worker; 
}
void workmap_delete(workmap *_map,int taskid){
    _map->erase(taskid);
}

void workmap_clear(workmap *_map){
    _map->clear();
}

void work_ref_inc(work* _pw){
    _pw->currentlinker++;
   printf("current liner %d\n\n",_pw->currentlinker);
}

bool work_finish(work* _pw){
    return (_pw->currentlinker==_pw->totallinker);
}


void close_all_files(){
    struct rlimit rl;
    int    i;
    if(getrlimit(RLIMIT_NOFILE,&rl)<0){
        printf("getrlimit failed");
    }else{
        if(rl.rlim_max == RLIM_INFINITY)
            rl.rlim_max = 655360;
        for(i=3;i<rl.rlim_max;i++)
            close(i);
    }
}
