#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "errors.h"


const int ScreenWidth = 1024;
const int ScreenHeight = 768;


StatusCode BuildShader(const char* Path, GLenum ShaderType, GLuint& ShaderObject);
StatusCode LinkProgram(GLuint* ShaderObjects, int ShaderCount, GLuint& ProgramObject);


struct ShaderProgram
{
	GLuint ProgramID;

	StatusCode ComputeCompile(const char* ComputePath);
	StatusCode RasterizationCompile(const char* VertexPath, const char* FragmentPath);
};


template<typename DataLayoutStruct, GLenum Target>
struct StructuredBuffer
{
	GLuint BufferID;
	GLuint BindingPoint;

	void Initialize(DataLayoutStruct* Data, GLuint ArrayElements = 1)
	{
		glGenBuffers(1, &BufferID);
		glBindBuffer(Target, BufferID);
		glBufferData(Target, sizeof(DataLayoutStruct) * ArrayElements, Data, GL_STATIC_DRAW);
		glBindBuffer(Target, 0);
	}
	void Bind()
	{
		glBindBuffer(Target, BufferID);
	}
	void Bind(GLenum NewTarget)
	{
		glBindBuffer(NewTarget, BufferID);
	}
	void BindBlock()
	{
		glBindBufferBase(Target, this->BindingPoint, this->BufferID);
	}
};


template<typename DataLayoutStruct>
struct UniformBuffer : public StructuredBuffer<DataLayoutStruct, GL_UNIFORM_BUFFER>
{
	void AttachToBlock(ShaderProgram Program, const char* BlockName)
	{
		GLuint BlockIndex = glGetProgramResourceIndex(Program.ProgramID, GL_UNIFORM_BLOCK, BlockName);
		this->BindingPoint = BlockIndex;
		glUniformBlockBinding(Program.ProgramID, BlockIndex, this->BindingPoint);
#if DEBUG_BUILD
		std::cout << "UBO #" << this->BufferID << " is attached to block "
			<< BlockName << " via block index " << BlockIndex << " and binding point " << this->BindingPoint << "\n";
#endif
	}
};


template<typename DataLayoutStruct>
struct ShaderStorageBuffer : public StructuredBuffer<DataLayoutStruct, GL_SHADER_STORAGE_BUFFER>
{
	void AttachToBlock(ShaderProgram Program, const char* BlockName)
	{
		GLuint BlockIndex = glGetProgramResourceIndex(Program.ProgramID, GL_SHADER_STORAGE_BLOCK, BlockName);
		this->BindingPoint = BlockIndex;
		glShaderStorageBlockBinding(Program.ProgramID, BlockIndex, this->BindingPoint);
#if DEBUG_BUILD
		std::cout << "SSBO #" << this->BufferID << " is attached to block "
			<< BlockName << " via block index " << BlockIndex << " and binding point " << this->BindingPoint << "\n";
#endif
	}
};