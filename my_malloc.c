#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "my_malloc.h"
#include<assert.h>
void *head=NULL;
unsigned long dataSeg=0;


//split the current block into two, to improve spatial effiency 
void split(MemBlock *curr, size_t size) {
    MemBlock *blockdata= curr + 1;
    MemBlock *sptblock =(MemBlock *) ((char*)blockdata +size);
    sptblock->size = curr->size - size - Blocksize;
    sptblock->next = curr->next;
	if(curr->next){
		sptblock->next->prev=sptblock;
	} 
    sptblock->isfree =true;
    curr->size = size;
    curr->next = sptblock;
    sptblock->prev = curr;
}
//find the first block which is free and larger than the required size
//if curr=null, the use last to track where to locate the new block
MemBlock * find_ffBlock(MemBlock **last,size_t size){
	if(head==NULL){
		return NULL;
	}
	MemBlock *curr=head;
 while(curr!=NULL){
  if(curr->isfree && (curr->size >=size)){
     if(curr->size>size+Blocksize){split(curr,size);}
		return curr;
	}
 *last=curr;
 curr=curr->next;}
 return curr;
 }
 //find the best block which is free and is the minimum difference 
 MemBlock * find_bfBlock(MemBlock **last,size_t size){
	if(head==NULL){
		return NULL;
	}
	MemBlock* curr=head;
	MemBlock* bf=NULL;
	while(curr!=NULL){
		if(curr->isfree&&curr->size>=size){
			if(bf==NULL||bf->size>curr->size){
				bf=curr;
			}
      //if alredy is the best, jump out of the loop
      if(bf->size==size){break;}
		}
   *last=curr;
		curr=curr->next;
	}
 return bf;
 }
 //add new block, to extend the heap size
 MemBlock *addNewBlock(MemBlock *last,size_t size){
	MemBlock *newblock=sbrk(Blocksize+size);
	if(newblock==(void*)-1){
	return NULL;}
 if(last!=NULL){
 last->next=newblock;}
	newblock->size=size;
	dataSeg+=size+Blocksize;
	newblock->next=NULL;
	newblock->isfree=false;
    newblock->prev=last;
	return newblock;
}
 
 void * ff_malloc(size_t size){
  if(size == 0)
    return NULL;
MemBlock * alloc;
MemBlock *last;
//first time, add new memblock
  if(head==NULL){
	  alloc = addNewBlock(NULL,size);
     if(alloc==NULL){return NULL;}
			head=alloc;
      	}
  else{
  last=head;
  alloc = find_ffBlock(&last,size);
  if(alloc != NULL){
	 alloc->isfree=false;
  }else{
    alloc = addNewBlock(last,size);
      if(alloc==NULL){return NULL;}
     }
  }
	return alloc+1;
}
void * bf_malloc(size_t size){
  if(size == 0)
    return NULL;
MemBlock * alloc;
MemBlock *last;
//the first time to add new block
  if(head==NULL){
	  alloc = addNewBlock(NULL,size);
	 if (!alloc) {return NULL;}
			head=alloc;
  }
  else{
  last=head;
  alloc = find_bfBlock(&last,size);
  if(alloc != NULL){
  if(alloc->size>Blocksize+size){split(alloc,size);}
	 alloc->isfree=false;
  }else{
    alloc = addNewBlock(last,size);
	 if (!alloc) {
                return NULL;
            }
     }
  }
 //print();
	return alloc+1;
}
//merge the adjacent memblock
MemBlock* merge(MemBlock* b){
	if(b){
		b->size+=Blocksize+b->next->size;
   b->next=b->next->next;
	  if(b->next){
		  b->next->prev=b;
	  }
	}
	return b;
}

void all_free(void *ptr){
	if(ptr==NULL){
		return ;
	}
	MemBlock* b=(MemBlock*)ptr-1;
// assert(b->isfree==true);
	b->isfree=true;
 	if(b->prev&&b->prev->isfree){
		b=merge(b->prev);
	}
	if(b->next&&b->next->isfree){
		b=merge(b);
	}	
}
void ff_free(void *ptr){
	all_free(ptr);
 }
 void bf_free(void *ptr){
	all_free(ptr);
 
}
 unsigned long get_data_segment_size(){

  return dataSeg;
}
unsigned long get_data_segment_free_space_size(){
	unsigned long space=0;
	MemBlock* curr=head;
	while(curr!=NULL){
		if(curr->isfree){
			space+=curr->size+Blocksize;
		}
		curr=curr->next;
	}
	return space;
}

