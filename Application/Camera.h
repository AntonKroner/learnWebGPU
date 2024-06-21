#ifndef Camera_H_
#define Camera_H_

#include <stdbool.h>
#include <tgmath.h>
#include "webgpu.h"
#include "GLFW/glfw3.h"
#include "./linear/algebra.h"

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

Matrix4f Application_Camera_viewGet(Camera camera);
void Application_Camera_move(Camera camera[static 1], float x, float y);
void Application_Camera_activate(
  Camera camera[static 1],
  int button,
  int action,
  float x,
  float y);
void Application_Camera_zoom(Camera camera[static 1], float x, float y);

Matrix4f Application_Camera_viewGet(Camera camera) {
  float cx = cos(camera.angles.x);
  float sx = sin(camera.angles.x);
  float cy = cos(camera.angles.y);
  float sy = sin(camera.angles.y);
  Vector3f position =
    Vector_scale(exp(-camera.zoom), Vector3f_make(cx * cy, sx * cy, sy));
  return Matrix4f_lookAt(position, Vector3f_make(0, 0, 1.0f), Vector3f_fill(1.0f));
}
const float sensitivity = 0.01f;
const float scrollSensitivity = 0.1f;
void Application_Camera_move(Camera camera[static 1], float x, float y) {
  if (camera->dragging) {
    camera->angles.x = sensitivity * (-x - camera->position.x);
    camera->angles.y = sensitivity * (y - camera->position.y);
  }
}
void Application_Camera_activate(
  Camera camera[static 1],
  int button,
  int action,
  float x,
  float y) {
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
void Application_Camera_zoom(Camera camera[static 1], float /*x*/, float y) {
  camera->zoom += scrollSensitivity * y;
}

#endif // Camera_H_
