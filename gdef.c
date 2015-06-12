#include <stdlib.h>
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


bool workmap_find(workmap *_map,int taskid){
   return (_map->end() != _map->find(taskid)); 
}
void workmap_insert(workmap *_map,int taskid,int fd){
   (*_map)[taskid]=fd; 
}
void workmap_delete(workmap *_map,int taskid){
    _map->erase(taskid);
}
