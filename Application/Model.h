#ifndef Model_H_
#define Model_H_

#include <stdlib.h>
#include <stdio.h>
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"
#include "linear/algebra.h"

typedef struct {
    Vector3f position;
    Vector3f normal;
    Vector3f color;
    Vector2f uv;
} Model_Vertex;
typedef struct {
    Model_Vertex* vertices;
    size_t vertexCount;
} Model;

static void loadFile(
  void* /* ctx */,
  const char* filename,
  const int /* is_mtl */,
  const char* /* obj_filename */,
  char** buffer,
  size_t* length) {
  long string_size = 0, read_size = 0;
  FILE* handler = fopen(filename, "r");
  if (handler) {
    fseek(handler, 0, SEEK_END);
    string_size = ftell(handler);
    rewind(handler);
    *buffer = malloc(sizeof(char) * (string_size + 1));
    read_size = fread(*buffer, sizeof(char), (size_t)string_size, handler);
    (*buffer)[string_size] = '\0';
    if (string_size != read_size) {
      free(*buffer);
      *buffer = NULL;
    }
    fclose(handler);
  }
  *length = read_size;
}
Model Model_load(const char* const file, Vector3f offset) {
  tinyobj_shape_t* shapes = 0;
  tinyobj_material_t* materials = 0;
  tinyobj_attrib_t attributes;
  size_t shapesCount;
  size_t materialsCount;
  tinyobj_attrib_init(&attributes);
  Model result = { .vertexCount = 0, .vertices = 0 };
  const int loaded = tinyobj_parse_obj(
    &attributes,
    &shapes,
    &shapesCount,
    &materials,
    &materialsCount,
    file,
    loadFile,
    0,
    TINYOBJ_FLAG_TRIANGULATE);
  if (loaded == TINYOBJ_SUCCESS) {
    result.vertexCount = attributes.num_faces;
    result.vertices = calloc(attributes.num_faces, sizeof(Model_Vertex));
    for (size_t i = 0; attributes.num_faces > i; i++) {
      const tinyobj_vertex_index_t face = attributes.faces[i];
      result.vertices[i].color.components[0] = 1.0;
      result.vertices[i].color.components[1] = 1.0;
      result.vertices[i].color.components[2] = 1.0;
      result.vertices[i].position.components[0] =
        offset.components[0] + attributes.vertices[3 * face.v_idx];
      result.vertices[i].position.components[1] =
        offset.components[1] + -1 * attributes.vertices[3 * face.v_idx + 2];
      result.vertices[i].position.components[2] =
        offset.components[2] + attributes.vertices[3 * face.v_idx + 1];
      result.vertices[i].normal.components[0] = attributes.normals[3 * face.vn_idx];
      result.vertices[i].normal.components[1] =
        -1 * attributes.normals[3 * face.vn_idx + 2];
      result.vertices[i].normal.components[2] = attributes.normals[3 * face.vn_idx + 1];
      result.vertices[i].uv.components[0] = attributes.texcoords[2 * face.vt_idx];
      result.vertices[i].uv.components[1] = 1 - attributes.texcoords[2 * face.vt_idx + 1];
    }
    tinyobj_attrib_free(&attributes);
    if (shapes) {
      tinyobj_shapes_free(shapes, shapesCount);
    }
    if (materials) {
      tinyobj_materials_free(materials, materialsCount);
    }
  }
  return result;
}
void Model_unload(Model* model) {
  free(model->vertices);
  model->vertexCount = 0;
}

#endif // Model_H_
