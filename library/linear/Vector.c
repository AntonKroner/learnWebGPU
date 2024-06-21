#include "algebra.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

Vector* Vector_create(size_t length) {
  Vector* result = calloc(1, sizeof(*result));
  result->components = calloc(length, sizeof(*(result->components)));
  result->length = length;
  return result;
}
void Vector_destroy(Vector* vector) {
  free(vector->components);
  vector->components = 0;
  vector->length = 0;
  free(vector);
  vector = 0;
}
void Vector_fill(Vector vector[static 1], double value) {
  for (size_t n = 0; vector->length > n; n++) {
    vector->components[n] = value;
  }
}
void Vector_set(Vector vector[static 1], double values[static vector->length]) {
  memcpy(vector->components, values, vector->length * sizeof(*values));
}
[[nodiscard]] Vector* Vector_from(size_t length, double values[static length]) {
  Vector* result = Vector_create(length);
  Vector_set(result, values);
  return result;
}
double(Vector_inner)(Vector a[static 1], Vector b[static 1]) {
  double result = 0;
  for (size_t n = 0; a->length > n; n++) {
    result += a->components[n] * b->components[n];
  }
  return result;
}
double(Vector_norm)(Vector vector[static 1]) {
  return sqrt((Vector_inner)(vector, vector));
}
void(Vector_scale)(double scalar, Vector vector[static 1]) {
  for (size_t n = 0; vector->length > n; n++) {
    vector->components[n] *= scalar;
  }
}
void(Vector_add)(Vector a[static 1], Vector b[static 1], Vector result[static 1]) {
  for (size_t n = 0; a->length > n; n++) {
    result->components[n] = a->components[n] + b->components[n];
  }
}
void(Vector_subtract)(Vector a[static 1], Vector b[static 1], Vector* result) {
  for (size_t n = 0; a->length > n; n++) {
    result->components[n] = a->components[n] - b->components[n];
  }
}
void(Vector_transform)(Matrix matrix[static 1], Vector v[static 1], Vector* result) {
  for (size_t n = 0; v->length > n; n++) {
    for (size_t m = 0; matrix->columns > m; m++) {
      result->components[n] +=
        v->components[m] * matrix->elements[matrix->columns * n + m];
    }
  }
}
void Vector_differentiate(Vector input[static 1], Vector result[static 1]) {
  register const double height = 2 * M_PI / input->length;
  for (size_t n = 0; input->length > n; n++) {
    for (size_t m = 0; input->length > m; m++) {
      result->components[n] +=
        input->components[m] * 0.5 * pow(-1, m + n)
        * ((m - n) ? tan(M_PI_2 - ((input->length - (m - n)) * 0.5 * height)) : 0);
    }
  }
}
void(Vector_print)(Vector vector[static 1]) {
  printf("[");
  if (vector->length) {
    printf(" %g", vector->components[0]);
    for (size_t n = 1; vector->length > n; n++) {
      printf(", %g", vector->components[n]);
    }
  }
  printf(" ]\n");
}
