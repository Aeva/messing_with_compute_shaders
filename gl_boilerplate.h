#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
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


template<GLenum Target>
struct StructuredBuffer
{
	GLuint BufferID;
	std::vector<GLuint> BindingPoints;

	void Initialize(void* Data, size_t Bytes)
	{
		glGenBuffers(1, &BufferID);
		glBindBuffer(Target, BufferID);
		glBufferData(Target, Bytes, Data, GL_STATIC_DRAW);
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
	void BindBlock(int Attachment=0)
	{
		glBindBufferBase(Target, this->BindingPoints[Attachment], this->BufferID);
	}
};


struct UniformBuffer : public StructuredBuffer<GL_UNIFORM_BUFFER>
{
	void AttachToBlock(ShaderProgram Program, const char* BlockName)
	{
		int Attachment = BindingPoints.size();
		GLuint BlockIndex = glGetProgramResourceIndex(Program.ProgramID, GL_UNIFORM_BLOCK, BlockName);
		this->BindingPoints.push_back(BlockIndex);
		glUniformBlockBinding(Program.ProgramID, BlockIndex, this->BindingPoints[Attachment]);
#if DEBUG_BUILD
		std::cout << "UBO #" << this->BufferID << " is attached to block "
			<< BlockName << " via block index " << BlockIndex << " and binding point " << this->BindingPoints[Attachment] << "\n";
#endif
	}
};


struct ShaderStorageBuffer : public StructuredBuffer<GL_SHADER_STORAGE_BUFFER>
{
	void AttachToBlock(ShaderProgram Program, const char* BlockName)
	{
		int Attachment = BindingPoints.size();
		GLuint BlockIndex = glGetProgramResourceIndex(Program.ProgramID, GL_SHADER_STORAGE_BLOCK, BlockName);
		this->BindingPoints.push_back(BlockIndex);
		glShaderStorageBlockBinding(Program.ProgramID, BlockIndex, this->BindingPoints[Attachment]);
#if DEBUG_BUILD
		std::cout << "SSBO #" << this->BufferID << " is attached to block "
			<< BlockName << " via block index " << BlockIndex << " and binding point " << this->BindingPoints[Attachment] << "\n";
#endif
	}
};

