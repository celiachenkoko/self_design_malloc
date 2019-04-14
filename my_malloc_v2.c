#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include<pthread.h>
#include"my_malloc.h"
//global freehead
MemBlock *freehead=NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//local freehead
__thread MemBlock * nolock_freehead = NULL;
void split(MemBlock *curr, size_t size) {
    MemBlock *blockdata= curr + 1;
  MemBlock *sptblock;
   sptblock =(MemBlock *) ((char*)blockdata+size);
    sptblock->size = curr->size - size - Blocksize;
    sptblock->nextfree= curr->nextfree;
if(sptblock->nextfree){
	sptblock->nextfree->prevfree=sptblock;
}
	sptblock->prevfree= curr;
 	curr->nextfree=sptblock;
	curr->size=size;
}
void toremove(MemBlock * deleteblock,MemBlock **freeheader,size_t size){
if(deleteblock==NULL) return ;
if(deleteblock->size>size+Blocksize){split(deleteblock,size);}
if(deleteblock==(*freeheader)){
	(*freeheader)=deleteblock->nextfree;
	if(deleteblock->nextfree!=NULL){
		deleteblock->nextfree->prevfree=NULL;
	}
}
else{deleteblock->prevfree->nextfree=deleteblock->nextfree;
if(deleteblock->nextfree){
deleteblock->nextfree->prevfree=deleteblock->prevfree;}
}
}


 MemBlock *addNewBlock(size_t size){
	 MemBlock *  newblock=sbrk(size+Blocksize);
	 newblock->size=size;
	 newblock->nextfree=NULL;
	 newblock->prevfree=NULL;
return newblock;}

 MemBlock *addNewBlock_nolock(size_t size){
	 pthread_mutex_lock(&lock);
	 MemBlock *  newblock=sbrk(size+Blocksize);
	 pthread_mutex_unlock(&lock);
	 newblock->size=size;
	 newblock->nextfree=NULL;
	 newblock->prevfree=NULL;
return newblock;}

 MemBlock * find_bfBlock(size_t size){
	if(freehead==NULL){
		return NULL;
	}
	MemBlock* curr=freehead;
	MemBlock* bf=NULL;
	while(curr!=NULL){
		if(curr->size>=size){
			if(bf==NULL||bf->size>curr->size){
				bf=curr;
			}
			if(bf->size==size){break;}
		}
		curr=curr->nextfree;
	}
	toremove(bf,&freehead,size);
 return bf;
 }
 ///no lock
  MemBlock * find_nolock_bfBlock(size_t size){
	if(nolock_freehead==NULL){
		return NULL;
	}
	MemBlock* curr=nolock_freehead;
	MemBlock* bf=NULL;
	while(curr!=NULL){
		if(curr->size>=size){
			if(bf==NULL||bf->size>curr->size){
				bf=curr;
			}
			if(bf->size==size){break;}
		}
		curr=curr->nextfree;
	}
  toremove(bf,&nolock_freehead,size);
 return bf;
 }
 ////
 void  * ts_malloc_lock(size_t size){
	 if(size==0) return NULL;
	  pthread_mutex_lock(&lock);
      MemBlock * alloc=NULL;
	  if(freehead==NULL){
		  freehead=sbrk(Blocksize);
        alloc=addNewBlock(size);
	  }
	  else{
	  alloc=find_bfBlock(size);
	  if(alloc==NULL){
		  alloc=addNewBlock(size);
	  }
 }
 pthread_mutex_unlock(&lock);
 return alloc+1;
 }
 ////
void *ts_malloc_nolock(size_t size){
		 if(size==0) return NULL;
     MemBlock * alloc=NULL;
	  if(nolock_freehead==NULL){
     pthread_mutex_lock(&lock);
		  nolock_freehead=sbrk(Blocksize);
    pthread_mutex_unlock(&lock);
    alloc=addNewBlock_nolock(size);
	  }
	else{
	 alloc=find_nolock_bfBlock(size);
	  if(alloc==NULL){
		  alloc=addNewBlock_nolock(size);
	  }
 }

 return alloc+1;
}
 
 MemBlock* merge(MemBlock* curr){
	 if(curr->nextfree&&((char*)curr+curr->size+Blocksize==(char*)curr->nextfree)){
		 curr->size+=curr->nextfree->size+Blocksize;
		 curr->nextfree=curr->nextfree->nextfree;
		 if(curr->nextfree){
			 curr->nextfree->prevfree=curr;
		 }
	 }
	 return curr;
 }
void ts_free_lock(void * ptr){
	if(ptr==NULL) return ;
 pthread_mutex_lock(&lock);
	 MemBlock* b=(MemBlock*)ptr-1;
	 MemBlock* curr=freehead;
   //no free blocks
	if(curr==NULL){
		freehead=b;
		b->prevfree=NULL;
		b->nextfree=NULL;
   pthread_mutex_unlock(&lock);
		return;
	}
	while(curr&&curr->nextfree&&(char*)curr<(char*)b){
		curr=curr->nextfree;
	}

	//1
	if((char*)curr>(char*)b){
 //b prior then the freehead
	if(curr==freehead){
		freehead=b;
		b->prevfree=NULL;
		b->nextfree=curr;
   if(curr){
		curr->prevfree=b;}
	}
 // b behind freehead prior b
	else{
		b->prevfree=curr->prevfree;
		b->nextfree=curr;
		curr->prevfree->nextfree=b;
		curr->prevfree=b;
	}
	}
	//b behind the last free block
	else if((char*)curr<(char*)b){
		curr->nextfree=b;
		b->prevfree=curr;
		b->nextfree=NULL;
	}
	//merge if address adjacent
	if(b->prevfree&&((char*)b->prevfree+(b->prevfree->size)+Blocksize==(char*)b)){
   b=merge(b->prevfree);
	}
	if(b->nextfree&&((char*)b+b->size+Blocksize==(char*)b->nextfree)){
		b=merge(b);
	}
	 pthread_mutex_unlock(&lock);
}

void ts_free_nolock(void *ptr){
	if(ptr==NULL) return ;
	 MemBlock* b=(MemBlock*)ptr-1;
	 MemBlock* curr=nolock_freehead;
	if(curr==NULL){
		nolock_freehead=b;
		b->prevfree=NULL;
		b->nextfree=NULL;
		return;
	}
	while(curr&&curr->nextfree&&(char*)curr<(char*)b){
		curr=curr->nextfree;
	}

	//1
	if((char*)curr>(char*)b){
	if(curr==nolock_freehead){
		nolock_freehead=b;
		b->prevfree=NULL;
		b->nextfree=curr;
   if(curr){
		curr->prevfree=b;}
	}
	else{
		b->prevfree=curr->prevfree;
		b->nextfree=curr;
		curr->prevfree->nextfree=b;
		curr->prevfree=b;
	}
	}
	//2
	else if((char*)curr<(char*)b){
		curr->nextfree=b;
		b->prevfree=curr;
		b->nextfree=NULL;
	}
	//merge if address adjacent
	if(b->prevfree&&((char*)b->prevfree+(b->prevfree->size)+Blocksize==(char*)b)){
		b=merge(b->prevfree);
	}
	if(b->nextfree&&((char*)b+b->size+Blocksize==(char*)b->nextfree)){
		b=merge(b);
	}
	}
