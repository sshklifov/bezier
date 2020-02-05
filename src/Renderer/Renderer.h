#ifndef SETUP_INCLUDED
#define SETUP_INCLUDED

#include "glad.h"
#include <GLFW/glfw3.h>

GLFWwindow* CreateRenderer(int width, int height);
void DestroyRenderer(GLFWwindow* window);
void SwapBuffers(GLFWwindow* window, void* fb);

#endif
