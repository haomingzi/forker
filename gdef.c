#include <stdlib.h>
#include <stdio.h>
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

void work_ref_inc(work* _pw){
    _pw->currentlinker++;
   printf("current liner %d\n\n",_pw->currentlinker);
}

bool work_finish(work* _pw){
    return (_pw->currentlinker==_pw->totallinker);
}
