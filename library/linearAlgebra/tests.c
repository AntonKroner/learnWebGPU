#include <stdlib.h>
#include <stdio.h>
#include "linearAlgebra.h"

int main() {
  const Vector2 r22 = Vector2_make(1.2, 5.3);
  const Vector3 r1 = Vector3_make(1, 0, 0);
  const Vector3 r2 = Vector3_make(0, 1, 0);
  const Vector3 r3 = Vector_cross(r1, r2);
  const Vector3 r4 = Vector_add(r1, r2);
  printf("r4 = ");
  Vector_print(r4);
  printf("[1, 0, 0] x [0, 1, 0] = ");
  Vector3_print(r3);
  printf("normalize([2, 2, 0]) =");
  Vector3_print(Vector3_normalize(Vector3_make(2, 2, 0)));

  return EXIT_SUCCESS;
}
