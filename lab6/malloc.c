/****************************************************************
 * Tyler Nguyen
 * cs360 - lab 6
 * malloc.c
 * Details : malloc.c will define the procedures jmalloc, 
			   jfree, jcalloc, and jrealloc. Malloc will
			   utilize the sbrk() system call to obtain memory
			   to dole out to the user.
****************************************************************/
#include "malloc.h"
#include <unistd.h>
#include <stdio.h>
#include <strings.h>

/* Free List Struct */
typedef struct flist {
	int size;  // size holds the size of the chunk 
	struct flist *next;  // next node
} *Flist;

/* Global Variable */
Flist head = NULL;


/******************** PROCEDURE DEFINITIONS ********************/
/* jmalloc */
void *jmalloc(unsigned int size){
	void *ptr, *alloc_ptr, *free_ptr;
	Flist chunk;

	size = (size + 7 + 8) & -8;  // allign size to 8 bytes

	/* we have available chunks to check, find chunk with enough size */
	if(head != NULL){
		/* iterate through the freelist */
		chunk = head;
		while(1){
			ptr = chunk;
			/* if we find an available chunk */
			if(size <= (chunk->size)){  
				ptr = chunk;  // save ptr to use this chunk
				break;
			}
			/* we are at last node and size doesn't fit */
			/* we have to call sbrk to make a new chunk */
			else if(chunk->next == NULL){
				Flist tmpChunk;
				if(size > 8192){  // user called malloc with a size greater than 8192
					ptr = sbrk(size);
					*(int*)ptr = size;
					return ptr+8;  // return the whole chunk
				}
				else{  // size is under 8192
					ptr = sbrk(8192);
					void* tmpPtr = ptr+size;  // increment pointer
					tmpChunk = (Flist)tmpPtr;
					tmpChunk->size = 8192 - size;  // calculate the remaining size
				}

				// append new chunk to end of list 
				tmpChunk->next = NULL;  
				chunk->next = tmpChunk;

				// return new chunk(tmpChunk) 
				*(int*)ptr = size;  // new heap will be at location of ptr
				return ptr + 8;
			}
			else{  // proceed to next node
				chunk = chunk->next;
			}
		}
	}
	/* If this is the first call to malloc */
	else if(head == NULL){
		/* add chunk to freelist */
		if(size > 8192){  // user called malloc with a size greater than 8192
			ptr = sbrk(size);
			*(int*)ptr = size;
			return ptr+8;  // return whole chunk
		}
		else{  // size is under 8192
			ptr = sbrk(8192);
			chunk = (Flist)ptr;  // typecast memory pointer to a chunk Node
			chunk->size = 8192;
			*(int*)ptr = size; // manually update size at pointer address
		}
		chunk->next = NULL;
		head = chunk;  // set head to chunk
		ptr = chunk;
	}

	/*********************** ALLOCATE THE MEMORY *************************/
	// work on the free_ptr, which is the remainder to be put back on freeList
	free_ptr = (void*)chunk;
	int remainderSize = *(int*)ptr - size;  // calculate the remaining size

	/* if we want to return the whole chunk (remainder = 0) */
	if(remainderSize <= 0){ 
		if(chunk->next!=NULL){  // if there is a node after current chunk
			*(int*)free_ptr = size;
			return free_ptr+8;
		}
		else if(head == chunk && chunk->next!=NULL){ // first node
			head = chunk->next;
			*(int*)free_ptr = size;
			return free_ptr+8;
		}
		else if(chunk->next == NULL && head->next == NULL){ // if only chunk
			head = NULL;  // reset head and return whole chunk
			*(int*)free_ptr = size;
			return free_ptr+8;
		}
		else{  // last chunk
			chunk->next = NULL;
			*(int*)free_ptr = size;
			return free_ptr+8;
		}
	}
	free_ptr = free_ptr + size;  // inc free_ptr by size bytes
	Flist new_chunk = (Flist)free_ptr;  // new_chunk is the remainder
	new_chunk->size = remainderSize;  // update new_chunk size

	// work on alloc_ptr, which is the pointer to be returned
	alloc_ptr = ptr;
	*(int*)alloc_ptr = size;  // add size to padding
	return alloc_ptr+8;
}

/* jfree */
void jfree(void *ptr){
	// back up 8 bytes to get chunk & size
	ptr = ptr-8;

	// create chunk
	Flist chunk = (Flist)ptr;

	// if head is null, return 
	if(head == NULL){
		return;
	}
	// if head is not null, add chunk to beginning of list 
	Flist tmp = head;
	head = chunk;
	chunk->next = tmp;
	return;
}

/* jcalloc */
void *jcalloc(unsigned int nmemb, unsigned int size){
	void *ret = jmalloc(nmemb*size);
	bzero(ret, nmemb*size);  //zero all of the bytes
	return ret;
}

/* jrealloc */
void *jrealloc(void *ptr, unsigned int size){
	void *ret = jmalloc(size);
	bcopy(ptr, ret, size);  // copy over all of the bytes from ptr
	jfree(ptr);
	return ret;
}
