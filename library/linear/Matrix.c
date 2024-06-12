#include "algebra.h"
#include <stdlib.h>
#include <stdio.h>
#include <tgmath.h>

Matrix* Matrix_create(size_t rows, size_t columns) {
  Matrix* result = calloc(1, sizeof(*result));
  result->elements = calloc(rows * columns, sizeof(*(result->elements)));
  result->rows = rows;
  result->columns = columns;
  return result;
}
void Matrix_destroy(Matrix* matrix) {
  free(matrix->elements);
  matrix->elements = 0;
  free(matrix);
}
void Matrix_fill(Matrix matrix[static 1], double value) {
  for (size_t n = 0; matrix->rows * matrix->columns > n; n++) {
    matrix->elements[n] = value;
  }
}
void Matrix_setDiagonal(Matrix matrix[static 1], double value) {
  for (size_t n = 0; matrix->rows > n; n++) {
    matrix->elements[matrix->columns * n + n] = value;
  }
}
void Matrix_add(Matrix a[static 1], Matrix b[static 1], Matrix result[static 1]) {
  for (size_t n = 0; a->rows > n; n++) {
    for (size_t m = 0; a->columns > m; m++) {
      result->elements[a->rows * n + m] =
        a->elements[a->rows * n + m] + b->elements[a->rows * n + m];
    }
  }
}
void Matrix_multiply(Matrix a[static 1], Matrix b[static 1], Matrix result[static 1]) {
  for (size_t n = 0; a->rows > n; n++) {
    for (size_t m = 0; b->columns > m; m++) {
      for (size_t k = 0; a->columns > k; k++) {
        result->elements[a->rows * n + m] +=
          a->elements[a->rows * n + k] * b->elements[k * b->columns + m];
      }
    }
  }
}
void Matrix_differentiate(Matrix result[static 1]) {
  register const double height = 2 * M_PI / result->columns;
  for (size_t n = 0; result->columns > n; n++) {
    for (size_t m = 0; result->columns > m; m++) {
      result->elements[n * result->rows + m] =
        0.5 * pow(-1, m + n)
        * ((m - n) ? tan(M_PI_2 - ((double)(result->columns - (m - n)) * 0.5 * height)) : 0);
    }
  }
  return;
}
void Matrix_print(Matrix matrix[static 1]) {
  for (size_t n = 0; matrix->rows > n; n++) {
    for (size_t m = 0; matrix->columns > m; m++) {
      printf("%g, ", matrix->elements[matrix->rows * n + m]);
    }
    printf("\n");
  }
}
