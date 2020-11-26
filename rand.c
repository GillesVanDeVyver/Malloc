#include <stdlib.h>
#include <math.h>

#define MAX 1024 * 1
#define MIN 4

int request() {
  double k = (log((double) MAX) / MIN);

  double r = ((double)(rand() % (int)(k*10000))) / 10000;

  int size = (int)((double) MAX / exp(r));

  return size;

}