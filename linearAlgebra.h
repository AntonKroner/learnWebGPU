#ifndef linearAlgebra_H_
#define linearAlgebra_H_
#include <stdio.h>

typedef struct {
    float elements[16];
} Matrix4;
typedef struct {
    float components[4];
} Vector4;

Matrix4 Matrix4_fill(float value);
Matrix4 Matrix4_diagonal(float value);
Matrix4 Matrix4_transpose(Matrix4 matrix);
Matrix4 Matrix4_add(Matrix4 a, Matrix4 b);
Matrix4 Matrix4_multiply(Matrix4 a, Matrix4 b);
void Matrix4_print(Matrix4 matrix);
Vector4 Vector4_from(float values[static 4]);
Vector4 Vector4_fill(float value);
Vector4 Vector4_scale(float scalar, Vector4 vector);
Vector4 Vector4_add(Vector4 a, Vector4 b);
Vector4 Vector4_transform(Matrix4 matrix, Vector4 v);
void Vector4_print(Vector4 vector);

Matrix4 Matrix4_fill(float value) {
  Matrix4 result = { 0 };
  for (size_t n = 0; 16 > n; n++) {
    result.elements[n] = value;
  }
  return result;
}
Matrix4 Matrix4_diagonal(float value) {
  Matrix4 result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    result.elements[4 * n + n] = value;
  }
  return result;
}
Matrix4 Matrix4_transpose(Matrix4 matrix) {
  Matrix4 result = { 0 };
  result.elements[0] = matrix.elements[0];
  result.elements[1] = matrix.elements[4];
  result.elements[2] = matrix.elements[8];
  result.elements[3] = matrix.elements[12];
  result.elements[4] = matrix.elements[1];
  result.elements[5] = matrix.elements[5];
  result.elements[6] = matrix.elements[9];
  result.elements[7] = matrix.elements[13];
  result.elements[8] = matrix.elements[2];
  result.elements[9] = matrix.elements[6];
  result.elements[10] = matrix.elements[10];
  result.elements[11] = matrix.elements[14];
  result.elements[12] = matrix.elements[3];
  result.elements[13] = matrix.elements[7];
  result.elements[14] = matrix.elements[11];
  result.elements[15] = matrix.elements[15];
  return result;
}
Matrix4 Matrix4_add(Matrix4 a, Matrix4 b) {
  Matrix4 result = { 0 };
  for (size_t n = 0; 16 > n; n++) {
    result.elements[n] = a.elements[n] + b.elements[n];
  }
  return result;
}
Matrix4 Matrix4_multiply(Matrix4 a, Matrix4 b) {
  Matrix4 result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    for (size_t m = 0; 4 > m; m++) {
      for (size_t k = 0; 4 > k; k++) {
        result.elements[4 * n + m] += a.elements[4 * n + k] * b.elements[k * 4 + m];
      }
    }
  }
  return result;
}
void Matrix4_print(Matrix4 matrix) {
  for (size_t n = 0; 4 > n; n++) {
    for (size_t m = 0; 4 > m; m++) {
      printf("%f, ", matrix.elements[4 * n + m]);
    }
    printf("\n");
  }
}
Vector4 Vector4_from(float values[static 4]) {
  Vector4 result = {
    .components = {values[0], values[1], values[2], values[3]}
  };
  return result;
}
Vector4 Vector4_fill(float value) {
  Vector4 result = {
    .components = {value, value, value, value}
  };
  return result;
}
Vector4 Vector4_scale(float scalar, Vector4 vector) {
  Vector4 result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    result.components[n] = vector.components[n] * scalar;
  }
  return result;
}
Vector4 Vector4_add(Vector4 a, Vector4 b) {
  Vector4 result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    result.components[n] = a.components[n] + b.components[n];
  }
  return result;
}
float Vector4_inner(Vector4 a, Vector4 b) {
  float result = 0;
  for (size_t n = 0; 4 > n; n++) {
    result += a.components[n] * b.components[n];
  }
  return result;
}
Vector4 Vector4_transform(Matrix4 matrix, Vector4 v) {
  Vector4 result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    for (size_t m = 0; 4 > m; m++) {
      result.components[n] += v.components[m] * matrix.elements[4 * n + m];
    }
  }
  return result;
}
void Vector4_print(Vector4 vector) {
  printf("[ ");
  for (size_t n = 0; 4 > n; n++) {
    printf("%f, ", vector.components[n]);
  }
  printf(" ]\n");
}
#endif // linearAlgebra_H_
