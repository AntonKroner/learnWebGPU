#include "algebra.h"
#include <stdio.h>
#include <stdbool.h>
#include <tgmath.h>
#include <float.h>

Vector2 Vector2_make(double x, double y) {
  Vector2 result = {
    .components = {x, y}
  };
  return result;
}
Vector2f Vector2f_make(float x, float y) {
  Vector2f result = {
    .components = {x, y}
  };
  return result;
}
Vector3 Vector3_make(double x, double y, double z) {
  Vector3 result = {
    .components = {x, y, z}
  };
  return result;
}
Vector3f Vector3f_make(float x, float y, float z) {
  Vector3f result = {
    .components = {x, y, z}
  };
  return result;
}
Vector4 Vector4_make(double x, double y, double z, double w) {
  Vector4 result = {
    .components = {x, y, z, w}
  };
  return result;
}
Vector4f Vector4f_make(float x, float y, float z, float w) {
  Vector4f result = {
    .components = {x, y, z, w}
  };
  return result;
}
#define VectorN_fill(N)                         \
  Vector##N Vector##N##_fill(double value) {    \
    Vector##N result = { .components = 0 };     \
    for (size_t i = 0; N > i; i++) {            \
      result.components[i] = value;             \
    }                                           \
    return result;                              \
  }                                             \
  Vector##N##f Vector##N##f_fill(float value) { \
    Vector##N##f result = { .components = 0 };  \
    for (size_t i = 0; N > i; i++) {            \
      result.components[i] = value;             \
    }                                           \
    return result;                              \
  }
#define VectorN_from(N)                                    \
  Vector##N Vector##N##_from(double values[static N]) {    \
    Vector##N result = { .components = { 0 } };            \
    for (size_t i = 0; N > i; i++) {                       \
      result.components[i] = values[i];                    \
    }                                                      \
    return result;                                         \
  }                                                        \
  Vector##N##f Vector##N##f_from(float values[static N]) { \
    Vector##N##f result = { .components = { 0 } };         \
    for (size_t i = 0; N > i; i++) {                       \
      result.components[i] = values[i];                    \
    }                                                      \
    return result;                                         \
  }
#define VectorN_scale(N)                                               \
  Vector##N Vector##N##_scale(double scalar, Vector##N vector) {       \
    Vector##N result = { 0 };                                          \
    for (size_t n = 0; N > n; n++) {                                   \
      result.components[n] = vector.components[n] * scalar;            \
    }                                                                  \
    return result;                                                     \
  }                                                                    \
  Vector##N##f Vector##N##f_scale(float scalar, Vector##N##f vector) { \
    Vector##N##f result = { 0 };                                       \
    for (size_t n = 0; N > n; n++) {                                   \
      result.components[n] = vector.components[n] * scalar;            \
    }                                                                  \
    return result;                                                     \
  }
#define VectorN_add(N)                                            \
  Vector##N Vector##N##_add(Vector##N a, Vector##N b) {           \
    Vector##N result = { 0 };                                     \
    for (size_t n = 0; N > n; n++) {                              \
      result.components[n] = a.components[n] + b.components[n];   \
    }                                                             \
    return result;                                                \
  }                                                               \
  Vector##N##f Vector##N##f_add(Vector##N##f a, Vector##N##f b) { \
    Vector##N##f result = { 0 };                                  \
    for (size_t n = 0; N > n; n++) {                              \
      result.components[n] = a.components[n] + b.components[n];   \
    }                                                             \
    return result;                                                \
  }
#define VectorN_subtract(N)                                            \
  Vector##N Vector##N##_subtract(Vector##N a, Vector##N b) {           \
    Vector##N result = { 0 };                                          \
    for (size_t n = 0; N > n; n++) {                                   \
      result.components[n] = a.components[n] - b.components[n];        \
    }                                                                  \
    return result;                                                     \
  }                                                                    \
  Vector##N##f Vector##N##f_subtract(Vector##N##f a, Vector##N##f b) { \
    Vector##N##f result = { 0 };                                       \
    for (size_t n = 0; N > n; n++) {                                   \
      result.components[n] = a.components[n] - b.components[n];        \
    }                                                                  \
    return result;                                                     \
  }
// detail: https://realtimecollisiondetection.net/blog/?p=89
#define VectorN_areEqual(N)                                                               \
  bool Vector##N##_areEqual(Vector##N a, Vector##N b) {                                   \
    bool result = true;                                                                   \
    for (size_t n = 0; result && N > n; n++) {                                            \
      if (                                                                                \
        fabs(a.components[n] - b.components[n])                                           \
        >= DBL_EPSILON * fmax(1.0, fmax(fabs(a.components[n]), fabs(b.components[n])))) { \
        result = false;                                                                   \
      }                                                                                   \
    }                                                                                     \
    return result;                                                                        \
  }                                                                                       \
  bool Vector##N##f_areEqual(Vector##N##f a, Vector##N##f b) {                            \
    bool result = true;                                                                   \
    for (size_t n = 0; result && N > n; n++) {                                            \
      if (                                                                                \
        fabs(a.components[n] - b.components[n])                                           \
        >= FLT_EPSILON                                                                    \
             * fmax(1.0f, fmax(fabs(a.components[n]), fabs(b.components[n])))) {          \
        result = false;                                                                   \
      }                                                                                   \
    }                                                                                     \
    return result;                                                                        \
  }
#define VectorN_inner(N)                                     \
  double Vector##N##_inner(Vector##N a, Vector##N b) {       \
    double result = 0;                                       \
    for (size_t n = 0; N > n; n++) {                         \
      result += a.components[n] * b.components[n];           \
    }                                                        \
    return result;                                           \
  }                                                          \
  float Vector##N##f_inner(Vector##N##f a, Vector##N##f b) { \
    float result = 0;                                        \
    for (size_t n = 0; N > n; n++) {                         \
      result += a.components[n] * b.components[n];           \
    }                                                        \
    return result;                                           \
  }
#define VectorN_norm(N)                                \
  double Vector##N##_norm(Vector##N vector) {          \
    return sqrt((Vector##N##_inner)(vector, vector));  \
  }                                                    \
  float Vector##N##f_norm(Vector##N##f vector) {       \
    return sqrt((Vector##N##f_inner)(vector, vector)); \
  }
#define VectorN_normalize(N)                            \
  Vector##N Vector##N##_normalize(Vector##N v) {        \
    const float norm = sqrt(Vector##N##_inner(v, v));   \
    Vector##N result = { .components = { 0 } };         \
    for (size_t n = 0; N > n; n++) {                    \
      result.components[n] = v.components[n] / norm;    \
    }                                                   \
    return result;                                      \
  }                                                     \
  Vector##N##f Vector##N##f_normalize(Vector##N##f v) { \
    const float norm = sqrt(Vector##N##f_inner(v, v));  \
    Vector##N##f result = { .components = { 0 } };      \
    for (size_t n = 0; N > n; n++) {                    \
      result.components[n] = v.components[n] / norm;    \
    }                                                   \
    return result;                                      \
  }
#define VectorN_transform(N)                                                  \
  Vector##N Vector##N##_transform(Matrix##N matrix, Vector##N v) {            \
    Vector##N result = { 0 };                                                 \
    for (size_t n = 0; N > n; n++) {                                          \
      for (size_t m = 0; N > m; m++) {                                        \
        result.components[n] += v.components[m] * matrix.elements[N * n + m]; \
      }                                                                       \
    }                                                                         \
    return result;                                                            \
  }                                                                           \
  Vector##N##f Vector##N##f_transform(Matrix##N##f matrix, Vector##N##f v) {  \
    Vector##N##f result = { 0 };                                              \
    for (size_t n = 0; N > n; n++) {                                          \
      for (size_t m = 0; N > m; m++) {                                        \
        result.components[n] += v.components[m] * matrix.elements[N * n + m]; \
      }                                                                       \
    }                                                                         \
    return result;                                                            \
  }
#define VectorN_print(N)                         \
  void Vector##N##_print(Vector##N vector) {     \
    printf("[ %g", vector.components[0]);        \
    for (size_t n = 1; N > n; n++) {             \
      printf(", %g", vector.components[n]);      \
    }                                            \
    printf(" ]\n");                              \
  }                                              \
  void Vector##N##f_print(Vector##N##f vector) { \
    printf("[ %g", vector.components[0]);        \
    for (size_t n = 1; N > n; n++) {             \
      printf(", %g", vector.components[n]);      \
    }                                            \
    printf(" ]\n");                              \
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
Vector3f Vector3f_cross(Vector3f v, Vector3f w) {
  Vector3f result = { 0 };
  result.components[0] =
    v.components[1] * w.components[2] - v.components[2] * w.components[1];
  result.components[1] =
    v.components[2] * w.components[0] - v.components[0] * w.components[2];
  result.components[2] =
    v.components[0] * w.components[1] - v.components[1] * w.components[0];
  return result;
}
VectorN_fill(2) VectorN_fill(3) VectorN_fill(4) VectorN_from(2) VectorN_from(3)
  VectorN_from(4) VectorN_scale(2) VectorN_scale(3) VectorN_scale(4) VectorN_add(2)
    VectorN_add(3) VectorN_add(4) VectorN_subtract(2) VectorN_subtract(3)
      VectorN_subtract(4) VectorN_areEqual(2) VectorN_areEqual(3) VectorN_areEqual(4)
        VectorN_inner(2) VectorN_inner(3) VectorN_inner(4) VectorN_norm(2) VectorN_norm(3)
          VectorN_norm(4) VectorN_normalize(2) VectorN_normalize(3) VectorN_normalize(4)
            VectorN_transform(3) VectorN_transform(4) VectorN_print(2) VectorN_print(3)
              VectorN_print(4)
