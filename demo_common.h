#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define FAST_DIV_ROUND_UP(LHS, RHS) ((LHS + RHS - 1) / RHS)


const int ScreenWidth = 1024;
const int ScreenHeight = 768;


bool BuildShader(const char* Path, GLenum ShaderType, GLuint& ShaderObject);
bool LinkProgram(GLuint* ShaderObjects, int ShaderCount, GLuint& ProgramObject);


void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3], const GLfloat Up[3]);
void ViewMatrix(GLfloat Out[16], const GLfloat Origin[3], const GLfloat Focus[3]);
void PerspectiveMatrix(GLfloat Out[16]);
void PrintMatrix(GLfloat Matrix[16], const char* MatrixName);


struct ShaderProgram
{
	GLuint ProgramID;

	bool ComputeCompile(const char* ComputePath);
	bool RasterizationCompile(const char* VertexPath, const char* FragmentPath);
};


struct DrawArraysIndirectCommand
{
	GLuint VertexCount;
	GLuint InstanceCount;
	GLuint First;
	GLuint BaseInstance;
};
