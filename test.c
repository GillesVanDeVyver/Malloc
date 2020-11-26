#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "dlmall.h"




void main(){ //for testing
    sanity(0);
    int *ptr;
    ptr = (int*) dalloc(sizeof(int));
    *ptr = 7;
    printf("Stored %d at %p\n",*ptr, ptr);
    sanity(0);
    int *ptr2;
    ptr2 = (int*) dalloc(sizeof(int));
    *ptr2=9;
    printf("Stored %d at %p\n",*ptr2, ptr2);
    sanity(0);
    dfree(ptr2);
    printf("Freed %p\n",ptr2);
    sanity(0);
    dfree(ptr);
    printf("Freed %p\n",ptr);
    sanity(2);
    printf("lenghtFlist %d\n",lenghtFlist());
}