#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include "errors.h"


const int ScreenWidth = 1024;
const int ScreenHeight = 768;


StatusCode BuildShader(const char* Path, GLenum ShaderType, GLuint& ShaderObject);
StatusCode LinkProgram(GLuint* ShaderObjects, int ShaderCount, GLuint& ProgramObject);


struct ShaderProgram
{
	GLuint ProgramID;
	std::vector<struct BindingPoint*> BindingPoints;

	StatusCode ComputeCompile(const char* ComputePath);
	StatusCode RasterizationCompile(const char* VertexPath, const char* FragmentPath);
};


struct Buffer
{
	GLuint BufferID;

	void Initialize(void* Data, size_t Bytes)
	{
		glCreateBuffers(1, &BufferID);
		glNamedBufferStorage(BufferID, Bytes, Data, 0);
	}
};


struct BindingPoint
{
	GLenum Target;
	GLuint BlockIndex;
	void Attach(Buffer &Blob)
	{
		glBindBufferBase(Target, BlockIndex, Blob.BufferID);
	}
};


struct UniformBlock : public BindingPoint
{
	void Initialize(ShaderProgram &Program, const char* BlockName)
	{
		Target = GL_UNIFORM_BUFFER;
		BlockIndex = glGetProgramResourceIndex(Program.ProgramID, GL_UNIFORM_BLOCK, BlockName);
		glUniformBlockBinding(Program.ProgramID, BlockIndex, BlockIndex);
#if DEBUG_BUILD
		std::cout << "UBO block " << BlockName << " attaches at " << BlockIndex << " in program " << Program.ProgramID << "\n";
#endif
	}
};


struct ShaderStorageBlock : public BindingPoint
{
	void Initialize(ShaderProgram &Program, const char* BlockName)
	{
		Target = GL_SHADER_STORAGE_BUFFER;
		BlockIndex = glGetProgramResourceIndex(Program.ProgramID, GL_SHADER_STORAGE_BLOCK, BlockName);
		glShaderStorageBlockBinding(Program.ProgramID, BlockIndex, BlockIndex);
#if DEBUG_BUILD
		std::cout << "SSBO block " << BlockName << " attaches at " << BlockIndex << " in program " << Program.ProgramID << "\n";
#endif
	}
};

