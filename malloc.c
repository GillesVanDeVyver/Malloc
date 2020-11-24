#define TRUE 1
#define FALSE 0
#define HEAD ( sizeof ( struct head ) )
#define MIN( size ) (((size)>(8))?( size) : ( 8 ) )
#define MAGIC(memory) ( ( struct head * )memory = 1)
#define HIDE( block ) ( void * ) ( ( struct head * ) block + 1)
#define ALIGN 8
#define ARENA (64 * 1024)


struct head {
    uint16_t bfree ;
    uint16_t bsize ; 
    uint16_t free ;
    uint16_t size ;
    struct head * next ;
    struct head * prev ;
};


struct head *after ( struct head * block ) {
return ( struct head * ) ( ) ;
}

struct head *before ( struct head * block ) {
return ( struct head * ) ( . . . . . ) ;
}

struct head * split ( struct head * block , int s i z e ) {
int rsize = . . . . .
block = >size = . . .
struct head *splt =
splt =>bsize = . . .
splt =>bfree = . . .
splt =>size = . . .
splt = >free = . . .
...
struct head *aft = . .
aft = >bsize = . . .
}
//blah blah