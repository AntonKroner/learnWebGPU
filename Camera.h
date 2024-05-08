#ifndef Camera_H_
#define Camera_H_

#include <stdbool.h>
#include <tgmath.h>
#include "./library/webgpu.h"
#include "./linearAlgebra.h"
#include "./library/glfw/include/GLFW/glfw3.h"

typedef struct {
    struct {
        float x;
        float y;
    } position;
    struct {
        float x;
        float y;
    } angles;
    float zoom;
    bool dragging;
} Camera;

Matrix4 Camera_viewGet(Camera camera);
void Camera_move(Camera camera[static 1], float x, float y);
void Camera_activate(Camera camera[static 1], int button, int action, float x, float y);
void Camera_zoom(Camera camera[static 1], float x, float y);

Matrix4 Camera_viewGet(Camera camera) {
  float cx = cos(camera.angles.x);
  float sx = sin(camera.angles.x);
  float cy = cos(camera.angles.y);
  float sy = sin(camera.angles.y);
  Vector3 position = Vector3_scale(exp(-camera.zoom), Vector3_make(cx * cy, sx * cy, sy));
  return Matrix4_lookAt(position, Vector3_make(0, 0, 1.0f), Vector3_fill(1.0f));
}
const float sensitivity = 0.01f;
const float scrollSensitivity = 0.1f;
void Camera_move(Camera camera[static 1], float x, float y) {
  if (camera->dragging) {
    camera->angles.x = sensitivity * (-x - camera->position.x);
    camera->angles.y = sensitivity * (y - camera->position.y);
  }
}
void Camera_activate(Camera camera[static 1], int button, int action, float x, float y) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    switch (action) {
      case GLFW_PRESS:
        camera->dragging = true;
        camera->position.x = -x;
        camera->position.y = y;
        break;
      case GLFW_RELEASE:
        camera->dragging = false;
        break;
    }
  }
}
void Camera_zoom(Camera camera[static 1], float x, float y) {
  camera->zoom += scrollSensitivity * y;
}

#endif // Camera_H_
