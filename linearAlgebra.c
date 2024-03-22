#include "linearAlgebra.h"
#include <stdio.h>
#include <tgmath.h>

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
Matrix4 Matrix4_orthographic(
  int left,
  int right,
  int bottom,
  int top,
  float near,
  float far) {
  Matrix4 result = { 0 };
  result.elements[0] = 2.0 / (right - left);
  result.elements[3] = -1.0 * (right + left) / (right - left);
  result.elements[5] = 2.0 / (top - bottom);
  result.elements[7] = -1.0 * (top + bottom) / (top - bottom);
  result.elements[10] = -2.0 / (far - near);
  result.elements[11] = -1.0 * (far + near) / (far - near);
  result.elements[15] = 1.0;
  return result;
}
Matrix4 Matrix4_perspective(float fov, float aspect, float near, float far) {
  Matrix4 result = { 0 };
  const float range = 1.0f / tan(M_PI * fov / 360.0f);
  result.elements[0] = range / aspect;
  result.elements[5] = aspect;
  result.elements[10] = -(far + near) / (far - near);
  result.elements[11] = -(2.0f * far * near) / (far - near);
  result.elements[14] = -1.0f;
  return result;
}
Matrix4 Matrix4_lookAt(Vector3 position, Vector3 target, Vector3 up) {
  const Vector3 forward =
    Vector3_normalize(Vector3_add(target, Vector3_scale(-1.0, position)));
  const Vector3 right = Vector3_normalize(Vector3_cross(forward, up));
  const Vector3 u = Vector3_normalize(Vector3_cross(right, forward));
  Matrix4 ori = Matrix4_diagonal(1.0);
  ori.elements[0] = right.components[0];
  ori.elements[1] = right.components[1];
  ori.elements[2] = right.components[2];
  ori.elements[4] = u.components[0];
  ori.elements[5] = u.components[1];
  ori.elements[6] = u.components[2];
  ori.elements[8] = -forward.components[0];
  ori.elements[9] = -forward.components[1];
  ori.elements[10] = -forward.components[2];
  Matrix4 p = Matrix4_diagonal(1.0);
  p.elements[3] = -position.components[0];
  p.elements[7] = -position.components[1];
  p.elements[11] = -position.components[2];
  return Matrix4_multiply(ori, p);
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
Vector3 Vector3_make(float x, float y, float z) {
  Vector3 result = {
    .components = {x, y, z}
  };
  return result;
}
Vector3 Vector3_from(float values[static 3]) {
  Vector3 result = {
    .components = {values[0], values[1], values[2]}
  };
  return result;
}
Vector3 Vector3_fill(float value) {
  Vector3 result = {
    .components = {value, value, value}
  };
  return result;
}
Vector3 Vector3_scale(float scalar, Vector3 vector) {
  Vector3 result = { 0 };
  for (size_t n = 0; 3 > n; n++) {
    result.components[n] = vector.components[n] * scalar;
  }
  return result;
}
Vector3 Vector3_add(Vector3 a, Vector3 b) {
  Vector3 result = { 0 };
  for (size_t n = 0; 3 > n; n++) {
    result.components[n] = a.components[n] + b.components[n];
  }
  return result;
}
float Vector3_inner(Vector3 a, Vector3 b) {
  float result = 0;
  for (size_t n = 0; 3 > n; n++) {
    result += a.components[n] * b.components[n];
  }
  return result;
}
Vector3 Vector3_normalize(Vector3 v) {
  const float length = sqrt(Vector3_inner(v, v));
  const Vector3 result = {
    .components = {v.components[0] / length,
                   v.components[1] / length,
                   v.components[2] / length}
  };
  return result;
}
Vector3 Vector3_cross(Vector3 v, Vector3 w) {
  Vector3 result = { 0 };
  result.components[0] =
    v.components[1] * w.components[2] - v.components[2] * w.components[1];
  result.components[1] =
    v.components[2] * w.components[0] - v.components[0] * w.components[2];
  result.components[2] =
    v.components[0] * w.components[1] - v.components[1] * w.components[0];
  return result;
}
void Vector3_print(Vector3 vector) {
  printf("[ ");
  for (size_t n = 0; 3 > n; n++) {
    printf("%f, ", vector.components[n]);
  }
  printf(" ]\n");
}
// #define linearAlgebraUnitTests
#ifdef linearAlgebraUnitTests
  #include <stdlib.h>

int main() {
  const Vector3 r1 = Vector3_make(1, 0, 0);
  const Vector3 r2 = Vector3_make(0, 1, 0);
  const Vector3 r3 = Vector3_cross(r1, r2);
  printf("[1, 0, 0] x [0, 1, 0] = ");
  Vector3_print(r3);
  printf("normalize([2, 2, 0]) =");
  Vector3_print(Vector3_normalize(Vector3_make(2, 2, 0)));

  return EXIT_SUCCESS;
}
#endif
