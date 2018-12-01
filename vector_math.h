#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define FAST_DIV_ROUND_UP(LHS, RHS) ((LHS + RHS - 1) / RHS)


void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3], const GLfloat Up[3]);
void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3]);

void PerspectiveMatrix(GLfloat Out[16]);

void PrintMatrix(GLfloat Matrix[16], const char* MatrixName);
