#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>


#define MINSIZE 8
#define TRUE 1
#define FALSE 0
#define HEAD ( sizeof ( struct head ) )
#define MIN( size ) (((size)>(MINSIZE))?( size) : ( MINSIZE ) )
#define MAGIC(memory) ( ( struct head* )memory - 1)
#define HIDE( block ) ( void* ) ( ( struct head* ) block + 1)
#define ALIGN 8
#define ARENA (64 * 1024)


struct head {
    uint16_t bfree ;
    uint16_t bsize ; 
    uint16_t free ;
    uint16_t size ;
    struct head *next ;
    struct head *prev ;
};


struct head *after ( struct head *block ) {
    return ( struct head* ) ((char*) block + block->size + HEAD) ;
}

struct head *before ( struct head *block ) {
    return ( struct head* ) ((char*) block - block->bsize - HEAD) ;
}

// function does not insert the new splt block => should be done explicilty afterwards
struct head *split ( struct head *block , int size ) {
    int rsize = (block->size-size-HEAD);
    block->size = rsize;

    struct head *splt = after(block);
    splt->bsize = rsize;
    splt->bfree = block->free;
    splt->size = size;
    splt->free = block->free;

    struct head *aft = after(splt);
    aft->bsize = size;

    return splt;
}

struct head *arena = NULL;
struct head *new () {
    if ( arena != NULL) {
        printf ("one arena already allocated \n");
        return NULL;
    }


    // using mmap, but could have used sbrk
    struct head *new = mmap(NULL, ARENA, 
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
    
    if ( new == MAP_FAILED) {
        printf ( "mmap failed : error %d\n" , errno ) ;
        return NULL;
    }

    /* make room for head and dummy */
    uint size = ARENA - 2*HEAD;
    new->bfree = FALSE;
    new->bsize = 0;
    new->free= TRUE;
    new->size = size;
    struct head *sentinel = after (new);
    /*  only touch the status fields */
    sentinel->bfree = TRUE;
    sentinel->bsize = size;
    sentinel->free = FALSE;
    sentinel->size = 0;
    sentinel->next = NULL;

    /* this is the only arena we have */
    arena = ( struct head*) new ;

}


struct head *flist ;


int lenghtFlist(){
    struct head *currBlock = flist;
    int count = 0;
    while (currBlock != NULL){
        count++;
        currBlock = currBlock->next;
    }
    return count;
}


int printflist(){
    struct head *currBlock = flist;
    printf("printing the free list\n");
    //printf("lenghtFlist %d\n",lenghtFlist());
    while (currBlock != NULL){
        sleep(1);
        printf("currBlock: %p\n",(char*)currBlock);
        printf("currBlock->size: %d\n",currBlock->size);
        printf("currBlock->bfree: %d\n",currBlock->bfree);
        printf("currBlock->free: %d\n",currBlock->free);
        printf("currBlock->next: %p\n",currBlock->next);
        printf("currBlock->prev: %p\n",currBlock->prev);
        currBlock = currBlock->next;
    }
}

int printflisttemp(){
    struct head *currBlock = flist;
    printf("printing the free list\n");
    //printf("lenghtFlist %d\n",lenghtFlist());
    while (currBlock != NULL){
        printf("currBlock: %p\n",(char*)currBlock);
        printf("currBlock->size: %d\n",currBlock->size);
        printf("currBlock->bfree: %d\n",currBlock->bfree);
        printf("currBlock->free: %d\n",currBlock->free);
        printf("currBlock->next: %p\n",currBlock->next);
        printf("currBlock->prev: %p\n",currBlock->prev);
        currBlock = currBlock->next;
    }
}




int sizeOK(int size){
    if ((size % ALIGN != 0) || (size < MINSIZE) )
        return 0;
    return 1;
}

void checkFreeList(){
    struct head *currBlock = flist;
    while (currBlock != NULL){
        struct head *next = currBlock->next;
        if (next !=NULL){
            assert(currBlock==next->prev);
        }
        assert(currBlock->free==TRUE);
        assert(sizeOK(currBlock->size));
        currBlock = next;
    }
}


void sanity(int verbose){
    if (verbose==1 || verbose == 2){
        printf("___START SANITY CHECK___\n");
    }
    if (flist == NULL) { 
        flist = new();
        assert(flist!=NULL);
        // if (flist==NULL){
        //     printf("flist==NULL\n");
        //     abort();
        // }
    }

    if (verbose==1){
        printf("flist: %p\n",flist);
        printf("lenghtFlist %d\n",lenghtFlist());
    }
    if (verbose==2){
        printflist();
    }
    struct head *currBlock = arena;
    int sentinelReached = FALSE;
    while (!sentinelReached){
        if (currBlock->size==0){
            sentinelReached = TRUE;
            if (verbose==1){
                printf("sentinel block\n");
            }
            assert(currBlock->size ==0);
        }
        if (verbose==1){
            printf("currBlock: %p\n",(char*)currBlock);
            printf("currBlock->size: %d\n",currBlock->size);
            printf("currBlock->bfree: %d\n",currBlock->bfree);
            printf("currBlock->free: %d\n",currBlock->free);
            printf("currBlock->next: %p\n",currBlock->next);
            printf("currBlock->prev: %p\n",currBlock->prev);
        }
        if (!sentinelReached){
            int *dataPtr = HIDE(currBlock);
            if (verbose==1){
                printf("Value:  %x\n", *dataPtr);
            }
        }
        struct head *afterBlock = after(currBlock);


        assert(!(sentinelReached==FALSE && afterBlock->bsize!=currBlock->size));

        // if (sentinelReached==FALSE && afterBlock->bsize!=currBlock->size){
        //     printf("afterBlock->bsize != currBlock->size\n");
        //     printf("afterBlock->bsize: %d\n",afterBlock->bsize);
        //     printf("currBlock->size: %d\n",currBlock->size);
        //     abort();
        // }

        //assert(!(sentinelReached==FALSE && afterBlock->bfree!=currBlock->free));

        if (sentinelReached==FALSE && afterBlock->bfree!=currBlock->free){
            printf("currBlock: %p\n",(char*)currBlock);
            printf("afterBlock: %p\n",(char*)afterBlock);
            printf("afterBlock->bfree != currBlock->free\n");
            printf("afterBlock->bfree: %d\n",afterBlock->bfree);
            printf("currBlock->free: %d\n",currBlock->free);
            abort();
        }

        currBlock = afterBlock;
    }
    
    checkFreeList();
    printflisttemp();
    if (verbose==1|| verbose == 2){
        printf("___END SANITY CHECK___\n");
    }
}

int printfBlocksSize(){
    struct head *currBlock = flist;
    printf("printing the free blocks sizes\n");
    printf("lenghtFlist %d\n",lenghtFlist());
    while (currBlock != NULL){
        printf("currBlock->size: %d\n",currBlock->size);
        currBlock = currBlock->next;
    }
}

int printfAvgBlocksSize(){
    struct head *currBlock = flist;
    int sizeCount = 0;
    int count = 0;
    while (currBlock != NULL){
        sizeCount+=currBlock->size;
        count++;
        currBlock = currBlock->next;
    }
    return sizeCount/count;
}

void detach ( struct head* block ) {
    block->free=FALSE;
    struct head *aft = after(block);
    aft->bfree =FALSE;
    if ( block->next != NULL){
        struct head *nextBlock = block->next;
        nextBlock->prev = block->prev;
    }

    if ( block->prev != NULL){
        struct head *prevBlock = block->prev;
        prevBlock->next = block->next;
    }
    else{
        flist = block->next;
    }
    block->next=NULL;
    block->prev=NULL;
}

void insert ( struct head *block ) {
    block->next = flist;
    block->prev = NULL;
    if ( flist!= NULL){
        struct head* firstBlock = flist;
        firstBlock->prev = block;
    }
    flist = block;
}

struct head *find (int size) {
    if (flist == NULL) { //If the freelist is empty, you need to create the arena (if not already created).
        flist = new();
        if (flist==NULL){
            return NULL;
        }
    }
    struct head *currBlock = flist;
    while (currBlock != NULL) {
        if (currBlock->size >size){
            if (currBlock->size-size>size+HEAD+8){
                struct head *splt = split(currBlock,currBlock->size-size-HEAD);
                insert(splt);
            }
            detach(currBlock);
            return currBlock;
        }
        else{
            currBlock = currBlock->next;
        }
    }
    return NULL;
}

int adjust(int request){
    int minRequest = MIN(request);
    int quotient = minRequest / ALIGN;
    int remainder = minRequest % ALIGN;
    if (remainder != 0)
        quotient++;
    if (quotient % 2 != 0) // even multiple of ALIGN, not sure with this is necessary
        quotient++;
    //printf("adjusted request %d\n",(ALIGN*quotient));
    return ALIGN*quotient;
}

void *dalloc ( size_t request ) {
    if ( request <= 0 ){
        return NULL;
    }
    int size = adjust (request) ;
    struct head *taken = find ( size ) ;
    if ( taken == NULL)
        return NULL;
    else
        return HIDE(taken);
}

void dfree ( void *memory ) {
    if (memory != NULL) {
        struct head *block = MAGIC(memory);
     //   printf("MAGIC(memory) %p\n",MAGIC(memory));

        struct head *aft = after(block); //will never be null bcs of sentinel
        block->free = TRUE;
        aft->bfree = TRUE;
        insert(block);
    }
return ;
}


