#include <stdlib.h>
#include <stdio.h>
#include <tgmath.h>
#include <stdbool.h>
#include <float.h>
#include "algebra.h"

static inline bool areEqual(const double a, const double b) {
  return fabs(a - b) <= DBL_EPSILON * fmax(1.0, fmax(fabs(a), fabs(b)));
}
void vectorSpaceDefinitions() {
  bool error = false;
  const Vector3 zeroes = Vector3_fill(0);
  const Vector3 u = Vector3_make(0.3, 56.123, 1233333333);
  const Vector3 v = Vector3_make(erf(56632), 888888, 9);
  const Vector3 w = Vector3_make(0.0003, 687971, M_PI * M_E);
  if (!Vector_areEqual(Vector_add(u, Vector_add(v, w)), Vector_add(Vector_add(u, v), w))) {
    printf("Vector3 addition is not associative.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_add(u, v), Vector_add(v, u))) {
    printf("Vector3 addition is not commutative.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_add(u, zeroes), u)) {
    printf("Vector3 addition does not satisy the identity property.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_add(v, Vector_scale(-1.0, v)), zeroes)) {
    printf("Vector3 addition does not satisy the inverse elements property.\n");
    error = true;
  }
  if (!Vector_areEqual(
        Vector_scale(5.123, Vector_scale(9999888.12, v)),
        Vector_scale(5.123 * 9999888.12, v))) {
    printf("Vector3 scalar multiplication is not compatible with field "
           "multiplication.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_scale(1.0, v), v)) {
    printf("Vector3 scalar multiplication does not satisy the identity property.\n");
    error = true;
  }
  if (!Vector_areEqual(
        Vector_scale(5.123, Vector_add(u, v)),
        Vector_add(Vector_scale(5.123, u), Vector_scale(5.123, v)))) {
    printf("Vector3 scalar multiplication is not distributive with respect to vector "
           "addition.\n");
    error = true;
  }
  if (!Vector_areEqual(
        Vector_scale(5.123 + 9999888.12, v),
        Vector_add(Vector_scale(5.123, v), Vector_scale(9999888.12, v)))) {
    printf("Vector3 scalar multiplication is not distributive with respect to vector "
           "addition.\n");
    error = true;
  }
  if (!error) {
    printf("All Vector3 vector space definitions fulfilled.\n");
  }
}
void innerProductSpaceProperties() {
  bool error = false;
  const Vector3 zeroes = Vector3_fill(0);
  const Vector3 u = Vector3_make(0.3, 56.123, 1233333333);
  const Vector3 v = Vector3_make(erf(56632), 888888, 9);
  const Vector3 w = Vector3_make(0.0003, 687971, M_PI * M_E);
  if (!(areEqual(Vector_inner(v, zeroes), Vector_inner(zeroes, v)))) {
    {
      printf("Vector3 inner product with zero vector is not zero.\n");
      error = true;
    }
  }
  if (Vector_inner(v, v) < 0 || cimag(Vector_inner(v, v)) > 0) {
    printf("Vector3 inner product with self is negative or non-real.\n");
    error = true;
  }
  if (!areEqual(
        Vector_inner(v, Vector_add(Vector_scale(5.123, u), Vector_scale(9999888.12, w))),
        5.123 * Vector_inner(v, u) + 9999888.12 * Vector_inner(v, w))) {
    printf("Vector3 something something not distributive.\n");
    error = true;
  }
  if (!error) {
    printf("All Vector3 inner product space properties fulfilled.\n");
  }
}
void crossProductProperties() {
  bool error = false;
  const Vector3 u = Vector3_make(0.3, 56.123, 1233333333);
  const Vector3 v = Vector3_make(erf(56632), 888888, 9);
  const Vector3 x = Vector3_make(1, 0, 0);
  const Vector3 y = Vector3_make(0, 1, 0);
  const Vector3 z = Vector3_make(0, 0, 1);
  if (!Vector_areEqual(Vector_cross(u, v), Vector_scale(-1.0, Vector_cross(v, u)))) {
    printf("Vector3 cross product is not anti-commutative.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_cross(x, y), z)) {
    printf("Vector3 x x y != z.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_cross(y, z), x)) {
    printf("Vector3 y x z != x.\n");
    error = true;
  }
  if (!Vector_areEqual(Vector_cross(z, x), y)) {
    printf("Vector3 z x x != y.\n");
    error = true;
  }
  if (!error) {
    printf("All Vector3 cross product properties fulfilled.\n");
  }
}

int main() {
  printf("normalize([2, 2, 0]) = ");
  Vector_print(Vector_normalize(Vector3_make(2, 2, 0)));
  vectorSpaceDefinitions();
  innerProductSpaceProperties();
  crossProductProperties();
  return EXIT_SUCCESS;
}
// gcc-13 -std=gnu2x tests.c VectorN.c Vector.c MatrixN.c Matrix.c -lm
