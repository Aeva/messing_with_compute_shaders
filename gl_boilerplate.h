#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <map>
#include "errors.h"


const int ScreenWidth = 1024;
const int ScreenHeight = 768;


struct ShaderPipeline
{
	GLuint PipelineID;
	std::map<GLenum, GLuint> Stages;
	std::vector<struct BindingPoint*> BindingPoints;

	StatusCode Setup(std::map<GLenum, std::string> Shaders);
	void Activate();
};


struct Buffer
{
	GLuint BufferID;
	void Initialize(void* Data, size_t Bytes);
	void Bind(GLenum Target, GLuint BindingIndex);
};

