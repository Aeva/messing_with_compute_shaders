#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define FAST_DIV_ROUND_UP(LHS, RHS) ((LHS + RHS - 1) / RHS)


const int ScreenWidth = 1024;
const int ScreenHeight = 768;


bool BuildShader(const char* Path, GLenum ShaderType, GLuint& ShaderObject);
bool LinkProgram(GLuint* ShaderObjects, int ShaderCount, GLuint& ProgramObject);


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
