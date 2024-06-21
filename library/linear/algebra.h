#ifndef linear_algebra_H_
#define linear_algebra_H_

#define LINEAR_ALGEBRA_VERSION "0.0.1"

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    double elements[16];
} Matrix4;
typedef struct {
    double elements[9];
} Matrix3;
typedef struct {
    double elements[4];
} Matrix2;
typedef struct {
    double components[4];
} Vector4;
typedef struct {
    double components[3];
} Vector3;
typedef struct {
    double components[2];
} Vector2;
typedef struct {
    float elements[16];
} Matrix4f;
typedef struct {
    float elements[9];
} Matrix3f;
typedef struct {
    float elements[4];
} Matrix2f;
typedef struct {
    float components[4];
} Vector4f;
typedef struct {
    float components[3];
} Vector3f;
typedef struct {
    float components[2];
} Vector2f;
typedef struct {
    double* restrict components;
    size_t length;
} Vector;
typedef struct {
    double* restrict elements;
    size_t rows;
    size_t columns;
} Matrix;

Matrix* Matrix_create(size_t rows, size_t columns);
void Matrix_destroy(Matrix* matrix);
void Matrix_fill(Matrix matrix[static 1], double value);
void Matrix_setDiagonal(Matrix matrix[static 1], double value);
void Matrix_add(Matrix a[static 1], Matrix b[static 1], Matrix result[static 1]);
void Matrix_multiply(Matrix a[static 1], Matrix b[static 1], Matrix result[static 1]);
void Matrix_differentiate(Matrix result[static 1]);
void Matrix_print(Matrix matrix[static 1]);

Vector* Vector_create(size_t length);
void Vector_destroy(Vector* vector);
void Vector_set(Vector vector[static 1], double values[static vector->length]);
Vector* Vector_from(size_t length, double values[static length]);
void(Vector_fill)(Vector vector[static 1], double value);
void(Vector_scale)(double scalar, Vector vector[static 1]);
void(Vector_add)(Vector a[static 1], Vector b[static 1], Vector result[static 1]);
void(Vector_subtract)(Vector a[static 1], Vector b[static 1], Vector* result);
double(Vector_inner)(Vector a[static 1], Vector b[static 1]);
double(Vector_norm)(Vector vector[static 1]);
void Vector_differentiate(Vector input[static 1], Vector result[static 1]);
void(Vector_transform)(Matrix matrix[static 1], Vector v[static 1], Vector* result);
void(Vector_print)(Vector vector[static 1]);

#define Vector_scale(a, b)    \
  _Generic(                   \
    (b),                      \
    Vector2: Vector2_scale,   \
    Vector3: Vector3_scale,   \
    Vector4: Vector4_scale,   \
    Vector2f: Vector2f_scale, \
    Vector3f: Vector3f_scale, \
    Vector4f: Vector4f_scale)((a), (b))
#define Vector_add(a, b)    \
  _Generic(                 \
    (a),                    \
    Vector2: Vector2_add,   \
    Vector3: Vector3_add,   \
    Vector4: Vector4_add,   \
    Vector2f: Vector2f_add, \
    Vector3f: Vector3f_add, \
    Vector4f: Vector4f_add)((a), (b))
#define Vector_subtract(a, b)    \
  _Generic(                      \
    (a),                         \
    Vector2: Vector2_subtract,   \
    Vector3: Vector3_subtract,   \
    Vector4: Vector4_subtract,   \
    Vector2f: Vector2f_subtract, \
    Vector3f: Vector3f_subtract, \
    Vector4f: Vector4f_subtract)((a), (b))
#define Vector_areEqual(a, b)    \
  _Generic(                      \
    (a),                         \
    Vector2: Vector2_areEqual,   \
    Vector3: Vector3_areEqual,   \
    Vector4: Vector4_areEqual,   \
    Vector2f: Vector2f_areEqual, \
    Vector3f: Vector3f_areEqual, \
    Vector4f: Vector4f_areEqual)((a), (b))
#define Vector_inner(a, b)    \
  _Generic(                   \
    (b),                      \
    Vector2: Vector2_inner,   \
    Vector3: Vector3_inner,   \
    Vector4: Vector4_inner,   \
    Vector2f: Vector2f_inner, \
    Vector3f: Vector3f_inner, \
    Vector4f: Vector4f_inner, \
    Vector *: Vector_inner)((a), (b))
#define Vector_norm(a)       \
  _Generic(                  \
    (a),                     \
    Vector2: Vector2_norm,   \
    Vector3: Vector3_norm,   \
    Vector4: Vector4_norm,   \
    Vector2f: Vector2f_norm, \
    Vector3f: Vector3f_norm, \
    Vector4f: Vector4f_norm, \
    Vector *: Vector_norm)(a)
#define Vector_normalize(a)       \
  _Generic(                       \
    (a),                          \
    Vector2: Vector2_normalize,   \
    Vector3: Vector3_normalize,   \
    Vector4: Vector4_normalize,   \
    Vector2f: Vector2f_normalize, \
    Vector3f: Vector3f_normalize, \
    Vector4f: Vector4f_normalize)(a)
#define Vector_transform(a, b)    \
  _Generic(                       \
    (b),                          \
    Vector2: Vector2_transform,   \
    Vector3: Vector3_transform,   \
    Vector4: Vector4_transform,   \
    Vector2f: Vector2f_transform, \
    Vector3f: Vector3f_transform, \
    Vector4f: Vector4f_transform)((a), (b))
#define Vector_print(a)       \
  _Generic(                   \
    (a),                      \
    Vector2: Vector2_print,   \
    Vector3: Vector3_print,   \
    Vector4: Vector4_print,   \
    Vector2f: Vector2f_print, \
    Vector3f: Vector3f_print, \
    Vector4f: Vector4f_print, \
    Vector *: Vector_print)(a)
#define Vector_cross(a, b) \
  _Generic((a), Vector3: Vector3_cross, Vector3f: Vector3f_cross)(a, b)

// Matrix4 double
Matrix4 Matrix4_fill(double value);
Matrix4 Matrix4_diagonal(double value);
Matrix4 Matrix4_transpose(Matrix4 matrix);
Matrix4 Matrix4_add(Matrix4 a, Matrix4 b);
Matrix4 Matrix4_multiply(Matrix4 a, Matrix4 b);
Matrix4 Matrix4_orthographic(
  int left,
  int right,
  int bottom,
  int top,
  double near,
  double far);
Matrix4 Matrix4_lookAt(Vector3 position, Vector3 target, Vector3 up);
Matrix4 Matrix4_perspective(double fov, double aspect, double near, double far);
void Matrix4_print(Matrix4 matrix);
// Matrix4f
Matrix4f Matrix4f_fill(float value);
Matrix4f Matrix4f_diagonal(float value);
Matrix4f Matrix4f_transpose(Matrix4f matrix);
Matrix4f Matrix4f_add(Matrix4f a, Matrix4f b);
Matrix4f Matrix4f_multiply(Matrix4f a, Matrix4f b);
Matrix4f Matrix4f_orthographic(
  int left,
  int right,
  int bottom,
  int top,
  float near,
  float far);
Matrix4f Matrix4f_lookAt(Vector3f position, Vector3f target, Vector3f up);
Matrix4f Matrix4f_perspective(float fov, float aspect, float near, float far);
void Matrix4f_print(Matrix4f matrix);

Vector4 Vector4_make(double x, double y, double z, double w);
Vector4f Vector4f_make(float x, float y, float z, float w);
Vector3 Vector3_make(double x, double y, double z);
Vector3f Vector3f_make(float x, float y, float z);
Vector2 Vector2_make(double x, double y);
Vector2f Vector2f_make(float x, float y);
Vector3 Vector3_cross(Vector3 v, Vector3 w);
Vector3f Vector3f_cross(Vector3f v, Vector3f w);

#define VectorN_declarations(N)                                             \
  Vector##N Vector##N##_from(double values[static N]);                      \
  Vector##N Vector##N##_fill(double value);                                 \
  Vector##N Vector##N##_scale(double scalar, Vector##N vector);             \
  Vector##N Vector##N##_add(Vector##N a, Vector##N b);                      \
  Vector##N Vector##N##_subtract(Vector##N a, Vector##N b);                 \
  bool Vector##N##_areEqual(Vector##N a, Vector##N b);                      \
  double Vector##N##_inner(Vector##N a, Vector##N b);                       \
  double Vector##N##_norm(Vector##N vector);                                \
  Vector##N Vector##N##_normalize(Vector##N v);                             \
  Vector##N Vector##N##_transform(Matrix##N matrix, Vector##N v);           \
  void Vector##N##_print(Vector##N vector);                                 \
  Vector##N##f Vector##N##f_from(float values[static N]);                   \
  Vector##N##f Vector##N##f_fill(float value);                              \
  Vector##N##f Vector##N##f_scale(float scalar, Vector##N##f vector);       \
  Vector##N##f Vector##N##f_add(Vector##N##f a, Vector##N##f b);            \
  Vector##N##f Vector##N##f_subtract(Vector##N##f a, Vector##N##f b);       \
  bool Vector##N##f_areEqual(Vector##N##f a, Vector##N##f b);               \
  float Vector##N##f_inner(Vector##N##f a, Vector##N##f b);                 \
  float Vector##N##f_norm(Vector##N##f vector);                             \
  Vector##N##f Vector##N##f_normalize(Vector##N##f v);                      \
  Vector##N##f Vector##N##f_transform(Matrix##N##f matrix, Vector##N##f v); \
  void Vector##N##f_print(Vector##N##f vector);

VectorN_declarations(2) VectorN_declarations(3) VectorN_declarations(4)
#undef VectorN_declarations

#endif // linear_algebra_H_
