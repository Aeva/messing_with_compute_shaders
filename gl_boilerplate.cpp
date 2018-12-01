#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "gl_boilerplate.h"


std::string ReadFile(const char* Path)
{
	std::ifstream File;
	File.open(Path);
	std::stringstream Reader;
	Reader << File.rdbuf();
	File.close();
	return Reader.str();
}


std::string CheckCompileStatus(GLuint ObjectId)
{
	GLint StatusValue;
	glGetShaderiv(ObjectId, GL_COMPILE_STATUS, &StatusValue);
	if (StatusValue == GL_FALSE)
	{
		GLint LogLength;
		glGetShaderiv(ObjectId, GL_INFO_LOG_LENGTH, &LogLength);
		if (LogLength)
		{
			std::string ErrorLog(LogLength, 0);
			glGetShaderInfoLog(ObjectId, LogLength, NULL, (char*) ErrorLog.data());
			return ErrorLog;
		}
		else
		{
			return std::string("An unknown error occured.");
		}
	}
	return std::string();
}


std::string CheckLinkStatus(GLuint ObjectId)
{
	GLint StatusValue;
	glGetProgramiv(ObjectId, GL_LINK_STATUS, &StatusValue);
	if (!StatusValue)
	{
		GLint LogLength;
		glGetProgramiv(ObjectId, GL_INFO_LOG_LENGTH, &LogLength);
		if (LogLength)
		{
			std::string ErrorLog(LogLength, 0);
			glGetProgramInfoLog(ObjectId, LogLength, NULL, (char*) ErrorLog.data());
			return ErrorLog;
		}
		else
		{
			return std::string("An unknown error occured.");
		}
	}
	return std::string();
}


StatusCode BuildShader(const char* Path, GLenum ShaderType, GLuint& ShaderObject)
{
	std::cout << "Building Shader: " << Path << '\n';
	std::string Source = ReadFile(Path);
	const GLchar* SourcePtr = Source.data();
	const GLint SourceSize = Source.size();

	ShaderObject = glCreateShader(ShaderType);
	glShaderSource(ShaderObject, 1, &SourcePtr, &SourceSize);
	glCompileShader(ShaderObject);

	std::string Error = CheckCompileStatus(ShaderObject);
	if (!Error.empty())
	{
		std::cout << Error << '\n';
		return StatusCode::FAIL;
	}
	return StatusCode::PASS;
}


StatusCode LinkProgram(GLuint* ShaderObjects, int ShaderCount, GLuint& ProgramObject)
{
	ProgramObject = glCreateProgram();
	for (int i=0; i<ShaderCount; ++i)
	{
		GLuint ShaderObject = ShaderObjects[i];
		glAttachShader(ProgramObject, ShaderObject);
	}
	glLinkProgram(ProgramObject);

	std::string Error = CheckLinkStatus(ProgramObject);
	if (!Error.empty())
	{
		std::cout << Error << '\n';
		return StatusCode::FAIL;
	}
	return StatusCode::PASS;
}


StatusCode ShaderProgram::ComputeCompile(const char* ComputePath)
{
	// compute shader
	GLuint ComputeShader;
	RETURN_ON_FAIL(BuildShader(ComputePath, GL_COMPUTE_SHADER, ComputeShader));
	return LinkProgram(&ComputeShader, 1, ProgramID);
}


StatusCode ShaderProgram::RasterizationCompile(const char* VertexPath, const char* FragmentPath)
{
	GLuint SplatShaders[2];
	RETURN_ON_FAIL(BuildShader(VertexPath, GL_VERTEX_SHADER, SplatShaders[0]));
	RETURN_ON_FAIL(BuildShader(FragmentPath, GL_FRAGMENT_SHADER, SplatShaders[1]));
	return LinkProgram(SplatShaders, 2, ProgramID);
}
