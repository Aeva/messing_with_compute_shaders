#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define FAST_DIV_ROUND_UP(LHS, RHS) ((LHS + RHS - 1) / RHS)

void IdentityMatrix(GLfloat Out[16]);
void MultiplyMatrices(GLfloat Out[16], GLfloat LHS[16], GLfloat RHS[16]);
void TransposeMatrix(GLfloat Out[16], GLfloat Original[16]);

void TranslationMatrix(GLfloat Out[16], GLfloat PositionX, GLfloat PositionY, GLfloat PositionZ);
void XRotationMatrix(GLfloat Out[16], GLfloat RotateX);
void YRotationMatrix(GLfloat Out[16], GLfloat RotateY);
void ZRotationMatrix(GLfloat Out[16], GLfloat RotateZ);
void ScaleMatrix(GLfloat Out[16], GLfloat ScaleX, GLfloat ScaleY, GLfloat ScaleZ);

void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3], const GLfloat Up[3]);
void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3]);

void OrthographicMatrix(GLfloat Out[16]);
void PerspectiveMatrix(GLfloat Out[16]);

void PrintMatrix(GLfloat Matrix[16], const char* MatrixName);

