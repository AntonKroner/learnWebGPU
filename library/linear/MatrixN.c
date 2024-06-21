#include "algebra.h"
#include <stdio.h>
#include <tgmath.h>

Matrix4f Matrix4f_fill(float value) {
  Matrix4f result = { 0 };
  for (size_t n = 0; 16 > n; n++) {
    result.elements[n] = value;
  }
  return result;
}
Matrix4f Matrix4f_diagonal(float value) {
  Matrix4f result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    result.elements[4 * n + n] = value;
  }
  return result;
}
Matrix4f Matrix4f_transpose(Matrix4f matrix) {
  Matrix4f result = { 0 };
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
Matrix4f Matrix4f_add(Matrix4f a, Matrix4f b) {
  Matrix4f result = { 0 };
  for (size_t n = 0; 16 > n; n++) {
    result.elements[n] = a.elements[n] + b.elements[n];
  }
  return result;
}
Matrix4f Matrix4f_multiply(Matrix4f a, Matrix4f b) {
  Matrix4f result = { 0 };
  for (size_t n = 0; 4 > n; n++) {
    for (size_t m = 0; 4 > m; m++) {
      for (size_t k = 0; 4 > k; k++) {
        result.elements[4 * n + m] += a.elements[4 * n + k] * b.elements[k * 4 + m];
      }
    }
  }
  return result;
}
void Matrix4f_print(Matrix4f matrix) {
  for (size_t n = 0; 4 > n; n++) {
    for (size_t m = 0; 4 > m; m++) {
      printf("%f, ", matrix.elements[4 * n + m]);
    }
    printf("\n");
  }
}
Matrix4f Matrix4f_orthographic(
  int left,
  int right,
  int bottom,
  int top,
  float near,
  float far) {
  Matrix4f result = { 0 };
  result.elements[0] = 2.0 / (right - left);
  result.elements[3] = -1.0 * (right + left) / (right - left);
  result.elements[5] = 2.0 / (top - bottom);
  result.elements[7] = -1.0 * (top + bottom) / (top - bottom);
  result.elements[10] = -2.0 / (far - near);
  result.elements[11] = -1.0 * (far + near) / (far - near);
  result.elements[15] = 1.0;
  return result;
}
Matrix4f Matrix4f_perspective(float fov, float aspect, float near, float far) {
  Matrix4f result = { 0 };
  const float range = 1.0f / tan(M_PI * fov / 360.0f);
  result.elements[0] = range / aspect;
  result.elements[5] = aspect;
  result.elements[10] = -(far + near) / (far - near);
  result.elements[11] = -(2.0f * far * near) / (far - near);
  result.elements[14] = -1.0f;
  return result;
}
Matrix4f Matrix4f_lookAt(Vector3f position, Vector3f target, Vector3f up) {
  const Vector3f forward =
    Vector3f_normalize(Vector3f_add(target, Vector3f_scale(-1.0, position)));
  const Vector3f right = Vector3f_normalize(Vector3f_cross(forward, up));
  const Vector3f u = Vector3f_normalize(Vector3f_cross(right, forward));
  Matrix4f ori = Matrix4f_diagonal(1.0);
  ori.elements[0] = right.components[0];
  ori.elements[1] = right.components[1];
  ori.elements[2] = right.components[2];
  ori.elements[4] = u.components[0];
  ori.elements[5] = u.components[1];
  ori.elements[6] = u.components[2];
  ori.elements[8] = -forward.components[0];
  ori.elements[9] = -forward.components[1];
  ori.elements[10] = -forward.components[2];
  Matrix4f p = Matrix4f_diagonal(1.0);
  p.elements[3] = -position.components[0];
  p.elements[7] = -position.components[1];
  p.elements[11] = -position.components[2];
  return Matrix4f_multiply(ori, p);
}
