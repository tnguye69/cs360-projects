//#include "malloc.h"
#include <unistd.h>
#include <stdio.h>

/* Free List Struct */
typedef struct flist {
	int size;  // size holds the size of the chunk 
	struct flist *next;  // next node
	struct flist *prev;  // prev node
} *Flist;

/* Global Variable */
Flist head = NULL;

/* jmalloc */
void *jmalloc2(unsigned int size){
	size = (size + 7 + 8) & -8;  // allign size to 8 bytes
	printf("size = %d\n", size);

	void *ptr, *alloc_ptr, *free_ptr;
	Flist chunk, prevChunk, nextChunk;
	nextChunk = NULL;
	prevChunk = NULL;

	if(head != NULL){  // we have available chunks to check, find chunk with enough size 
		/* iterate through the freelist */
		chunk = head;
		while(1){
			ptr = chunk;
			/* if we find an available chunk */
			if(size <= chunk->size){  // if we find an available chunk
				//if we use the FIRST chunk in the list
				if(chunk->prev==NULL && chunk->next!=NULL){
					prevChunk = NULL;  // don't save prevChunk
					nextChunk = chunk->next;  // save next chunk
					break;
				}
				//if we use the LAST chunk in the list
				else if(chunk->prev!=NULL && chunk->next==NULL){
					prevChunk = chunk->prev;  // save prevChunk
					nextChunk = NULL;  // dont save nextChunk
					break;
				}
				//if we use a MIDDLE chunk
				else if(chunk->prev!=NULL && chunk->next!=NULL){
					prevChunk = chunk->prev;
					nextChunk = chunk->next;
					break;
				}
				//if there's only one node
				else if(chunk->prev==NULL && chunk->next==NULL){
					break;
				}
			}
			/* we are at last node and size doesn't fit */
			/* we have to call sbrk to make a new chunk */
			else if(chunk->next == NULL){
				Flist tmpChunk;
				if(size > 8192){  // user called malloc with a size greater than 8192
					ptr = sbrk(size);
					*(long*)ptr = size;
					return ptr+8;  // return the whole chunk
				}
				else{  // size is under 8192
					ptr = sbrk(8192);
					void* tmpPtr = ptr+size;  // increment pointer
					tmpChunk = (Flist)tmpPtr;
					tmpChunk->size = 8192 - size;
				}

				// append new chunk to end of list 
				tmpChunk->next = NULL;  
				tmpChunk->prev = chunk;
				chunk->next = tmpChunk;
				
				// return new chunk 
				*(long*)ptr = size;
				return ptr + 8;
			}
			else{  // proceed to next node
				chunk = chunk->next;
			}
		}
	}
	else if(head == NULL){  // if it is the first call to malloc
		/* add chunk to freelist */
		if(size > 8192){  // user called malloc with a size greater than 8192
			ptr = sbrk(size);
			*(long*)ptr = size;
			return ptr+8;  // return whole chunk
		}
		else{  // size is under 8192
			ptr = sbrk(8192);
			chunk = (Flist)ptr;  // typecast memory pointer to a chunk Node
			chunk->size = 8192;
		}
		chunk->next = NULL;
		chunk->prev = NULL;
		head = chunk;  // set head to chunk
	}
		
	/************** ALLOCATE THE MEMORY ***************/
	
	// work on the free_ptr, which is the remainder to be put back on list
	free_ptr = (void*)ptr;
	int remainderSize = *(long*)ptr - size;  // calculate the remaining size
	if(remainderSize <= 0){  // if we want to give the whole chunk (remainder = 0)
		//if first chunk
		if(chunk->prev==NULL && chunk->next!=NULL){
			nextChunk = chunk->next;
			head = chunk->next;
			chunk->next = NULL;
			nextChunk->prev = NULL;
			*(long*)free_ptr = size;
			return free_ptr+8;
		}
		//if middle
		else if(chunk->prev!=NULL && chunk->next!=NULL){
			prevChunk = chunk->prev;
			nextChunk = chunk->next;
			prevChunk->next = nextChunk;
			nextChunk->prev = prevChunk;
			chunk->next = NULL;
			chunk->prev = NULL;
			*(long*)free_ptr = size;
			return free_ptr+8;
		}
		//if last
		else if(chunk->prev!=NULL && chunk->next==NULL){
			prevChunk = chunk->prev;
			prevChunk->next = NULL;
			chunk->prev = NULL;
			*(long*)free_ptr = size;
			return free_ptr+8;
		}
		// if only chunk
		else{ 
			chunk->next = NULL;
			chunk->prev = NULL;
			head = NULL;
			*(long*)free_ptr = size;
			return free_ptr+8;
		}
	}
	free_ptr = free_ptr + size;  // inc free_ptr by size bytes
	Flist new_chunk = (Flist)free_ptr;  // new_chunk is the remainder
	/* LINK LINK LINK*/
	if(prevChunk==NULL && nextChunk!=NULL){  // first node
		new_chunk->prev = NULL;

		new_chunk->next = nextChunk;
		nextChunk->prev = new_chunk;
		head = new_chunk;
	}
	else if(prevChunk!=NULL && nextChunk==NULL){ // last node
		new_chunk->prev = prevChunk;
		prevChunk->next = new_chunk;

		new_chunk->next = NULL;
	}
	else if(prevChunk!=NULL && nextChunk!=NULL){ // middle
		prevChunk->next = new_chunk;
		new_chunk->prev = prevChunk;

		nextChunk->prev = new_chunk;
		new_chunk->next = nextChunk;
	}
	else if(prevChunk==NULL && nextChunk==NULL){
		head = new_chunk;
	}
	
	// work on alloc_ptr, which is the pointer to be returned
	// and free return chunk
	alloc_ptr = ptr;
	*(long*)alloc_ptr = size;
	chunk->next = NULL;
	chunk->prev = NULL;

	new_chunk->size = remainderSize;  // update size
	return alloc_ptr+8;

	/*free_ptr = ptr;   // free_ptr holds chunk to be subtracted
	alloc_ptr = ptr;  //alloc_ptr is the chunk to be returned 

	free_ptr = free_ptr+size;  // inc free_ptr by size bytes
	free_chunk = free_ptr;   // add new node to list
	free_chunk->size = chunk->size - size;  // update size of the new node
	chunk->size = NULL;
	free_chunk->next = NULL;
	free_chunk->prev = NULL;
	head = free_chunk;
	printf("free_ptr value is 0x%x\n", free_chunk);
	printf("free_chunk size is %d\n", free_chunk->size);

	*(int*)alloc_ptr = size;
	printf("alloc_ptr value is %x\n", alloc_ptr);

	return alloc_ptr+8;*/
}

/* function to traverse through flist */
void traverse_flist(Flist chunk){
	Flist tmp;
	for(tmp = chunk; tmp != NULL; tmp = tmp->next){
		printf("Chunk at address %p\n", tmp);
		printf("Chunk->size = %d\n", (int)tmp->size);
		printf("Chunk->next = %p\n", tmp->next);
		printf("Chunk->prev = %p\n\n", tmp->prev);
	}
}

/* jfree */
void my_free(void *ptr){
	Flist chunk;

	// back up 8 bytes and get size of ptr
	ptr = ptr-8;
	int size = *(int*)ptr;

	// create chunk
	//chunk = (Flist)ptr;
	//chunk->size = size;

	/* if head is null, make head the new chunk */
	if(head == NULL){
		chunk = (Flist)ptr;
		chunk->size = size;
		chunk->next = NULL;
		chunk->prev = NULL;
		head = chunk;
		return;
	}
	/* if head is not null, add chunk to end of list */
	else{
		chunk = (Flist)ptr;
		chunk->size = size;
		// find chunk at the end of the list
		Flist tmpChunk = head;  // tmpChunk will stop at last node
		printf("head->size = %p\n", head);
		while(tmpChunk->next != NULL){
			tmpChunk = tmpChunk->next;
		}

		// append
		printf("tmpChunk->size = %d\n", tmpChunk->size);
		tmpChunk->next = chunk;
		chunk->size = size;
		chunk->prev = tmpChunk;
		chunk->next = NULL;
		return;
	}
}

int main(){
	int *check = jmalloc2(82);
	printf("check is %p\n", check);
	printf("check size is %d\n\n", check[-2]);
	//my_free(check);
	printf("head->size = %d\n", head->size);


	//int *check2 = jmalloc2(8);
	//printf("check2 is %p\n", check2);
	//printf("check2 size is %d\n\n", check2[-2]);
	//my_free(check2);
	//printf("head->size = %d\n", head->size);

	//int *check3 = jmalloc2(24);
	//printf("check3 is 0x%p\n", check3);
	//printf("check3 size is %d\n\n", check3[-2]);
	//my_free(check3);

	//my_free(check2);
	//my_free(check3);

	//long *check4 = jmalloc2(11000);
	//printf("check4 is %p\n", check4);
	//printf("check4 size is %d\n\n", check4[-2]);
	//my_free(check4);
	
	//long check4 = jmalloc2(8112);

	   /*
	   int i;
	   for(i=0; i < 16; i++){
	   printf("0x%x\n", check[i]);
	   }*/

	printf("head->size = %d\n", head->size);
	traverse_flist(head);
	return 0;
}
