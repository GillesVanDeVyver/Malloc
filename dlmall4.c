#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>//mmap() function
#include <errno.h>
#include "dlmall.h"

//#define start main
#define TRUE 1
#define FALSE 0

#define HEAD (sizeof(struct head))

#define TAKEN (sizeof(struct taken))
#define MARGIN (HEAD - TAKEN)

#define MIN(size) ((size)>(16)?(size):(16))

//Limit of the size of a block in order to split it
#define LIMIT(size) (MIN(0) + HEAD + size)

#define MAGIC(memory) ((struct head*)memory-1)
#define HIDE(block) (void*)((struct head*)block +1)
//#define HIDE(block) (void*)((struct taken*)block +1)
#define ALIGN 8
#define ARENA (64*1024)


/*int start(){
	//init();
	int* ptr = dalloc(104);
	
	printf("Ptr[1] contains %d\n", ptr[1]);
	
	int pid = getpid();

	printf("\n\n /proc/%d/maps \n\n", pid);
	char command[50];
	sprintf(command, "cat /proc/%d/maps", pid);
	system(command);
	return 0;
}*/

struct head{ //head represents the nodes in the linked list 
	uint16_t bfree;//2 bytes, the status of the block before
	uint16_t bsize;//the size of the block before
	uint16_t free;//the status of the block
	uint16_t size;//the size of the block
	struct head *next;//8 bytes pointer
	struct head *prev;//8 bytes pointer	
};


//For improvement
//only holds sizes and flags fields
struct taken{
    uint16_t bfree;//2 bytes, the status of the block before
    uint16_t bsize;//the size of the block before
    uint16_t free;//the status of the block
    uint16_t size;//the size of the block
};
//after function
//Argument: the pointer to a given block
//Return: the pointer to the block that is after the argument block
struct head *after(struct head *block){
    return (struct head*)((char *)block + block->size + HEAD);
}

//before function
//Argument: the pointer to a given block
//Return: the pointer to the block that is before the argument block
struct head *before(struct head *block){
	return (struct head*)((char *)block - block->bsize - HEAD);
}

//split a block
//Argument: a large enough block and the required size for the block
//Return: a pointer to the second block
struct head *split(struct head *block, int size){
	int rsize = block->size - size - HEAD; //remaining size
	block->size = rsize;
	
	struct head *splt = after(block);
	splt->bsize = rsize;
	splt->bfree = block->free;
	splt->size = size;
	splt->free = FALSE;
	
	struct head *aft = after(splt);
	aft->bsize = size;
	
	return splt;	
}

//create a new block
struct head *arena = NULL;

struct head *new(){
	if(arena != NULL){
		printf("one arena already allocated \n");
		return NULL;
	}

	//first argument of mmap is Null: kernel decide the starting 	mapping address
	struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE, 	MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if(new == MAP_FAILED){
		printf("mmap failed: error %d\n", errno);
		//return NULL;
	}

	int size = ARENA - 2*HEAD;//make room for head and dummy
	new->bfree = FALSE; //to prevent anyone from trying to merge it
	//with an non-existing block before
	new->bsize = 0;
	new->free = TRUE;
	new->size = size;

	struct head *sentinel = after(new);
	sentinel->bfree = new->free;//TRUE
	sentinel->bsize = size;
	sentinel->free = FALSE;
	sentinel->size = HEAD;//size will not be zero if the block exist

	arena = (struct head*)new;//the only arena that we have
	
	return new;
}

//the free list
struct head *flist;//pointer pointing to the first block of the free list

//to detach a block from the list
void detach(struct head *block){
	if(block->next != NULL && block->prev != NULL){ 
		block->prev->next = block->next;
		block->next->prev = block->prev;
		block->prev = NULL;
		block->next = NULL;
	}
	else if(block->prev != NULL){
		block->prev->next = NULL; //block->next == null or both null => error
		block->prev = NULL;		
	}
	else if(block->next != NULL){ 
		block->next->prev = NULL; // block->prev==null
		flist = block->next;
		block->next = NULL;
	}
	else{
		//block->next->prev = NULL;
		//block->prev->next = NULL;
		flist = NULL;
	}	
}

//to insert a new block in the list (flist)
//no return statement, only7 modify the free list

void insert(struct head *block){
	block->next = flist;
	block->prev = NULL;
	if(flist != NULL){
		flist->prev = block;
	}
	flist = block;//move the pointer pointing to the new start(block)
}

void orderInsert(struct head *block){
    if(flist== NULL){
        insert(block);
    }
    else{
        struct head *temp = flist;
        while(block->size > temp->size){
            temp = temp->next;
        }
        block->next = temp;
        block->prev = temp->prev;
    }
}

//2.3 allocate and free
//allocate a block
//same functionality as malloc

//adjust(): help function for dalloc
//functionality: find the suitable size
//input: request size
//output; suitable size
int adjust(size_t request){
	int minSize = MIN(request);
	if(minSize % ALIGN == 0){
		return minSize;
	}
	minSize = minSize - (minSize % ALIGN) + ALIGN;
	return minSize;
	
}

//find(): help function for dalloc
//functionality: find the block in the freelist with the adjusted size
//input: the adjusted size of the block 
//output: the found block
struct head *find(int size){
    struct head *temp = flist;
    while(temp != NULL){
	//for(struct head *temp = flist; temp->next != NULL; temp = temp->next){
        
        if(temp->size + MARGIN >= size){
		//if(temp->size >= size){
		detach(temp);
			if(temp->size >= LIMIT(size)){ //split it
				struct head *split1 = split(temp, size);
				//remaining part
				struct head *split2 = before(split1);
				//insert remaining
				insert(split2);
				//!change the status of split1: not free
				struct head *aft = after(split1);
				split1->free = FALSE;
				aft->bfree = FALSE;
				return split1;
												
			}
			
			//temp size bigger than required but smaller than LIMIT, no need to split
			else{
				struct head *aft1 = after(temp);
				temp->free = FALSE;
				aft1->bfree = FALSE;
				return temp;
			}
		}
		else{//if temp->size < size
			//continue;
			temp = temp->next;
		}
		
	}
	return NULL; //if no suitable block was found
}

//merge()
//input: a given block
//output: merge the block with adjacent blocks if free
struct head *merge(struct head *block){
	struct head *aft = after(block);
	if(block->bfree == TRUE){
		//unlink the block before
		struct head *bf = before(block);
		detach(bf);
		//calculate and set the total size of the merge blocks
		int totalSize = bf->size + block->size + HEAD;
		//update the block after the merged blocks
		bf->size = totalSize;
		bf->free = TRUE;
		aft->bfree = TRUE;
		aft->bsize = bf->size;
		//continue with the merged block
		block = bf;
	}
	if(aft->free){
		//unlink the block
		detach(aft);
		//calculate with the total size
		block->size = aft->size + block->size + HEAD;
		//update the block after the merged blocks
		aft = after(block);
		aft->bsize = block->size;
		
	}
	block->free = TRUE;
	aft->bfree = TRUE;
	return block;
}

void *dalloc(size_t request){
	if(request <= 0){
		return NULL;
	}
	int size = adjust(request);
	//printf("Requested size: %ld\n", request);
	//printf("Adjusted size: %d\n", size);
	struct head *taken = find(size);
	if(taken == NULL){ 
		return NULL;
	}
	else{ 
		//printf("Allocation is done\n");
		return HIDE(taken);
	}
}


//free a block
//!!!!!!!!!no merge is implemented for now
//input: given memory that needs to be freed
void dfree(void *memory){
	if(memory != NULL){
		struct head *block = MAGIC(memory);//Retrieve the header
		struct head *aft = after(block);
		block->free = TRUE;
		aft->bfree = TRUE;
        //block=merge(block);
		//insert(block);
        orderInsert(block);
	}
	return;	
}

//return the free list
struct head *returnFlist(){
	return flist;
}

//initiate the first block
void init(){
	struct head *firstBlock = new();
	insert(firstBlock);
}

int sizeOfFreeBlocks(){
    int count = 0;
    struct head *first = flist;
    while(first != NULL){
        if(first->free == TRUE){
            count = count + first->size;
            first = first->next;
        }
        else{
            first = first->next;
        }
    }
    return count;
}

int lengthOfFlist(){
    int count = 0;
    struct head *first = flist;
    while(first != NULL){
        if(first->free == TRUE){
            count++;
            first = first->next;
        }
    }
    return count;
}








 


