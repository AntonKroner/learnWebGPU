#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
// #include "./01basic3d/basic3d.h"
#include "./Application.h"

int main(int argc, char* argv[static argc + 1]) {
  printf(
    "glfw version: %d %d %d\n",
    GLFW_VERSION_MAJOR,
    GLFW_VERSION_MINOR,
    GLFW_VERSION_REVISION);
  int result = EXIT_FAILURE;
  extern char* optarg;
  int index = 0;
  int option = 0;
  int flag = 0;
  const struct option options[] = {
    {"input", required_argument,     0, 'i'},
    { "flag",       no_argument, &flag,   1},
    {      0,                 0,     0,   0}
  };
  while (option != EOF) {
    option = getopt_long(argc, argv, "", options, &index);
    switch (option) {
      case 0:
        break;
      case '?':
        printf("Error case.");
        break;
      case 'i':
        printf("input: %s\n", optarg);
    }
  }
  Application* application = Application_create();
  while (!Application_shouldClose(application)) {
    Application_render(application);
  }
  Application_destroy(application);
  return result;
}
