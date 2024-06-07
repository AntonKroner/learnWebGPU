#ifndef linearAlgebra_H_
#define linearAlgebra_H_

typedef struct {
    float elements[16];
} Matrix4;
typedef struct {
    float components[4];
} Vector4;
typedef struct {
    float components[3];
} Vector3;

#define Vector_add(a, b) \
  _Generic((a), Vector3: Vector3_add, Vector4: Vector4_add)((a), (b))

Matrix4 Matrix4_fill(float value);
Matrix4 Matrix4_diagonal(float value);
Matrix4 Matrix4_transpose(Matrix4 matrix);
Matrix4 Matrix4_add(Matrix4 a, Matrix4 b);
Matrix4 Matrix4_multiply(Matrix4 a, Matrix4 b);
Matrix4 Matrix4_orthographic(
  int left,
  int right,
  int bottom,
  int top,
  float near,
  float far);
Matrix4 Matrix4_lookAt(Vector3 position, Vector3 target, Vector3 up);
Matrix4 Matrix4_perspective(float fov, float aspect, float near, float far);
void Matrix4_print(Matrix4 matrix);
Vector4 Vector4_from(float values[static 4]);
Vector4 Vector4_fill(float value);
Vector4 Vector4_scale(float scalar, Vector4 vector);
Vector4 Vector4_add(Vector4 a, Vector4 b);
Vector4 Vector4_transform(Matrix4 matrix, Vector4 v);
void Vector4_print(Vector4 vector);
Vector3 Vector3_make(float x, float y, float z);
Vector3 Vector3_from(float values[static 3]);
Vector3 Vector3_fill(float value);
Vector3 Vector3_scale(float scalar, Vector3 vector);
Vector3 Vector3_add(Vector3 a, Vector3 b);
Vector3 Vector3_normalize(Vector3 v);
Vector3 Vector3_cross(Vector3 v, Vector3 w);
void Vector3_print(Vector3 vector);

#endif // linearAlgebra_H_
