#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "rand.h"
#include "dlmall3.h"


#define ROUNDS 50
#define LOOP 50
#define BUFFER 100

int main() {
  srand(1);
  void *buffer[BUFFER];
  for(int i = 0; i < BUFFER; i++) {
    buffer[i] = NULL;
  }


  for(int j = 0; j < ROUNDS; j++) {
    for(int i = 0; i < LOOP; i++) {
      // printf("ok\n");
      int index = rand() % BUFFER;
      if(buffer[index] != NULL) {
        // printf("freeing %p", buffer[index] );
        dfree(buffer[index]);
      }
      size_t size = (size_t)request();
      int *memory;
      memory = dalloc(size);
      // printf("requesting %ld\n",size);
      if(memory == NULL) {
        //printf("malloc failed (out of memory) \n");
        return 0;
      }
      else{
        // printf("ok3\n");
        buffer[index] = memory;
        *memory = 42;
        // printf("ok4\n");
      }
      printf("%d\n",avgBlocksSize());
      // sanity(1);
    }
  }
  return 0;
}
