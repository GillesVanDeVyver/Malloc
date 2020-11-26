#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "rand.h"
#include "dlmall.h"


#define ROUNDS 50
#define LOOP 50
#define BUFFER 10

int main() {
  void *buffer[BUFFER];
  for(int i = 0; i < BUFFER; i++) {
    buffer[i] = NULL;
  }
  for(int j = 0; j < ROUNDS; j++) {
    for(int i = 0; i < LOOP; i++) {
      int index = rand() % BUFFER;
      if(buffer[index] != NULL) {
      //  printf("freeing up %p\n",buffer[index]);
        dfree(buffer[index]);
        printf("%d\n",printfAvgBlocksSize());
      }
      size_t size = (size_t)request();
      int *memory;
      memory = dalloc(size);
      if(memory == NULL) {
        //printf("malloc failed (out of memory) \n");
        return 0;
      }
      else{
        buffer[index] = memory;
        *memory = 42;
      }
    }
  }
  return 0;
}
