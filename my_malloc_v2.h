#ifndef __MYMALLOC_H__
#define __MYMALLOC_H__
#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>

#define Blocksize sizeof(MemBlock)
struct _MemBlock{
	size_t size;
	struct _MemBlock *nextfree;
	struct _MemBlock *prevfree;
};
typedef struct _MemBlock MemBlock;
void split(MemBlock *curr, size_t size);
void toremove(MemBlock * delete,MemBlock **freeheader,size_t size);
MemBlock *addNewBlock(size_t size);
MemBlock * find_bfBlock(size_t size);
MemBlock * find_nolock_bfBlock(size_t size);
 void  * ts_malloc_lock(size_t size);
 void *ts_malloc_nolock(size_t size);
 MemBlock* merge(MemBlock* curr);
 void ts_free_lock(void * ptr);
 void ts_free_nolock(void *ptr);
 #endif