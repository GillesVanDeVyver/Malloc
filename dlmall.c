#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>


#define TRUE 1
#define FALSE 0
#define HEAD ( sizeof ( struct head ) )
#define MIN( size ) (((size)>(8))?( size) : ( 8 ) )
#define MAGIC(memory) ( ( struct head* )memory = 1)
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

    /* this is the only arena we have */
    arena = ( struct head*) new ;

}


struct head *flist ;
void detach ( struct head* block ) {
    block->free=FALSE;
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
            if (currBlock->size>2*8+2*HEAD){
                struct head *splt = split(currBlock,size);
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
    int quotient = request / ALIGN;
    int remainder = request % ALIGN;
    if (remainder != 0)
        remainder++;
    if (quotient % 2 != 0) // even multiple of ALIGN, not sure with this is necessary
        quotient++;
    return remainder*quotient;
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
        return taken;
}

void dfree ( void *memory ) {
    if (memory != NULL) {
        struct head *block = memory;
        struct head *aft = after(block);
        block->free = TRUE;
        aft->bfree = TRUE;
    }
return ;
}
