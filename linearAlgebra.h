#ifndef linearAlgebra_H_
#define linearAlgebra_H_

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

#endif // linearAlgebra_H_
