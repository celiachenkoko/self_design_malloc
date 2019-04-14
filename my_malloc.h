#ifndef __MYMALLOC_H__
#define __MYMALLOC_H__
#include <unistd.h>
#include <stdbool.h>
#define Blocksize sizeof(MemBlock)
struct _MemBlock{
	size_t size;
	struct _MemBlock *next;
	struct _MemBlock *prev;
	bool isfree;//see the state of each block
};
typedef struct _MemBlock MemBlock;
void split(MemBlock *curr, size_t size);
MemBlock * find_ffBlock(MemBlock **last,size_t size);
 MemBlock * find_bfBlock(MemBlock **last,size_t size);
 MemBlock *addNewBlock(MemBlock *last,size_t size);
void *ff_malloc(size_t size);
void * bf_malloc(size_t size);
void all_free(void *ptr);
void ff_free(void * ptr);
void bf_free(void * ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
#endif