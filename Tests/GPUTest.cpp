#include <stdlib.h>
#include "QPULib.h"


int data[4][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};

void vector_add(Ptr<Int> a, Ptr<Int> b, Ptr<Int> r) {
	*r = *a + *b;
}

int main() {
  // Construct kernel
  auto k = compile(vector_add);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<int> a(16), b(16), r(16);
  
  for (int i = 0; i < 16; i++) {
    a[i] = i + 1;
    b[i] = i + 1;
  }

  // Invoke the kernel and display the result
  k(&a, &b, &r);
  
  for (int i = 0; i < 16; i++)
    printf("%i + %i = %i\n", a[i], b[i], r[i]);
  
  return 0;
}
