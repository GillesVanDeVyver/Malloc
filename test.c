#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include "dlmall.h"


void main(){
    int *ptr = 0;
    ptr = (int*) malloc(1 * sizeof(int));
    printf("%p: %d\n",ptr, *ptr);
    free(ptr);
    *ptr = 0;
    ptr = (int*) dalloc(1 * sizeof(int));
    printf("%p: %d\n",ptr, *ptr);
    dfree(ptr);
}

