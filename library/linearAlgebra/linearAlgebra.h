#ifndef linearAlgebra_H_
#define linearAlgebra_H_

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
    float components[4];
} Vector4f;
typedef struct {
    float components[3];
} Vector3f;
typedef struct {
    float components[2];
} Vector2f;

// typedef struct {
//     float* components;
// } Vector;
// Vector Vector_add(Vector a, Vector b);

#define Vector_add(a, b)    \
  _Generic(                 \
    (a),                    \
    Vector2: Vector2_add,   \
    Vector3: Vector3_add,   \
    Vector4: Vector4_add,   \
    Vector2f: Vector2f_add, \
    Vector3f: Vector3f_add, \
    Vector4f: Vector4f_add)((a), (b))
#define Vector_scale(a, b)    \
  _Generic(                   \
    (b),                      \
    Vector2: Vector2_scale,   \
    Vector3: Vector3_scale,   \
    Vector4: Vector4_scale,   \
    Vector2f: Vector2f_scale, \
    Vector3f: Vector3f_scale, \
    Vector4f: Vector4f_scale)((a), (b))
#define Vector_inner(a, b)    \
  _Generic(                   \
    (b),                      \
    Vector2: Vector2_inner,   \
    Vector3: Vector3_inner,   \
    Vector4: Vector4_inner,   \
    Vector2f: Vector2f_inner, \
    Vector3f: Vector3f_inner, \
    Vector4f: Vector4f_inner)((a), (b))
#define Vector_transform(a, b)    \
  _Generic(                       \
    (b),                          \
    Vector2: Vector2_transform,   \
    Vector3: Vector3_transform,   \
    Vector4: Vector4_transform,   \
    Vector2f: Vector2f_transform, \
    Vector3f: Vector3f_transform, \
    Vector4f: Vector4f_transform)((a), (b))
#define Vector_normalize(a)       \
  _Generic(                       \
    (a),                          \
    Vector2: Vector2_normalize,   \
    Vector3: Vector3_normalize,   \
    Vector4: Vector4_normalize,   \
    Vector2f: Vector2f_normalize, \
    Vector3f: Vector3f_normalize, \
    Vector4f: Vector4f_normalize)(a)
#define Vector_print(a)       \
  _Generic(                   \
    (a),                      \
    Vector2: Vector2_print,   \
    Vector3: Vector3_print,   \
    Vector4: Vector4_print,   \
    Vector2f: Vector2f_print, \
    Vector3f: Vector3f_print, \
    Vector4f: Vector4f_print)(a)
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
Matrix4f Matrix4f_lookAt(Vector3 position, Vector3 target, Vector3 up);
Matrix4f Matrix4f_perspective(float fov, float aspect, float near, float far);
void Matrix4f_print(Matrix4f matrix);
// Vector4 double
Vector4 Vector4_make(double x, double y, double z, double w);
Vector4 Vector4_from(double values[static 4]);
Vector4 Vector4_fill(double value);
Vector4 Vector4_scale(double scalar, Vector4 vector);
Vector4 Vector4_add(Vector4 a, Vector4 b);
Vector4 Vector4_transform(Matrix4 matrix, Vector4 v);
void Vector4_print(Vector4 vector);
// Vector4f
Vector4f Vector4f_make(float x, float y, float z, float w);
Vector4f Vector4f_from(float values[static 4]);
Vector4f Vector4f_fill(float value);
Vector4f Vector4f_scale(float scalar, Vector4f vector);
Vector4f Vector4f_add(Vector4f a, Vector4f b);
Vector4f Vector4f_transform(Matrix4f matrix, Vector4f v);
void Vector4f_print(Vector4f vector);
// Matrix3 double
Matrix3 Matrix3_fill(double value);
Matrix3 Matrix3_diagonal(double value);
Matrix3 Matrix3_transpose(Matrix3 matrix);
Matrix3 Matrix3_add(Matrix3 a, Matrix3 b);
Matrix3 Matrix3_multiply(Matrix3 a, Matrix3 b);
void Matrix3_print(Matrix3 matrix);
// Matrix3f
Matrix3f Matrix3f_fill(float value);
Matrix3f Matrix3f_diagonal(float value);
Matrix3f Matrix3f_transpose(Matrix3f matrix);
Matrix3f Matrix3f_add(Matrix3f a, Matrix3f b);
Matrix3f Matrix3f_multiply(Matrix3f a, Matrix3f b);
void Matrix3f_print(Matrix3f matrix);
// Vector3 double
Vector3 Vector3_make(double x, double y, double z);
Vector3 Vector3_from(double values[static 3]);
Vector3 Vector3_fill(double value);
Vector3 Vector3_scale(double scalar, Vector3 vector);
Vector3 Vector3_add(Vector3 a, Vector3 b);
Vector3 Vector3_normalize(Vector3 v);
Vector3 Vector3_cross(Vector3 v, Vector3 w);
Vector3 Vector3_transform(Matrix3 matrix, Vector3 v);
void Vector3_print(Vector3 vector);
// Vector3f
Vector3f Vector3f_make(float x, float y, float z);
Vector3f Vector3f_from(float values[static 3]);
Vector3f Vector3f_fill(float value);
Vector3f Vector3f_scale(float scalar, Vector3f vector);
Vector3f Vector3f_add(Vector3f a, Vector3f b);
Vector3f Vector3f_normalize(Vector3f v);
Vector3f Vector3f_cross(Vector3f v, Vector3f w);
Vector3 Vector3_transform(Matrix3 matrix, Vector3 v);
void Vector3f_print(Vector3f vector);
// Vector2 double
Vector2 Vector2_make(double x, double y);
Vector2 Vector2_from(double values[static 2]);
Vector2 Vector2_fill(double value);
Vector2 Vector2_scale(double scalar, Vector2 vector);
Vector2 Vector2_add(Vector2 a, Vector2 b);
Vector2 Vector2_normalize(Vector2 v);
Vector2 Vector2_transform(Matrix3 matrix, Vector2 v);
void Vector2_print(Vector2 vector);
// Vector2f
Vector2f Vector2f_make(float x, float y);
Vector2f Vector2f_from(float values[static 2]);
Vector2f Vector2f_fill(float value);
Vector2f Vector2f_scale(float scalar, Vector2f vector);
Vector2f Vector2f_add(Vector2f a, Vector2f b);
Vector2f Vector2f_normalize(Vector2f v);
Vector2 Vector2_transform(Matrix3 matrix, Vector2 v);
void Vector2f_print(Vector2f vector);

#endif // linearAlgebra_H_
