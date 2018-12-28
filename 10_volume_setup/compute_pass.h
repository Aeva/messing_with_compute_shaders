#pragma once
#include "../errors.h"
#include "../gl_boilerplate.h"


namespace CullingPass
{
	StatusCode Setup();
	void Dispatch();
}


struct ScreenUniforms
{
	GLuint Width;
	GLuint Height;
};


struct Bounds
{
	GLfloat Center[3];
	GLfloat Extent[4];
};


struct ActiveRegion
{
	GLfloat StartDepth;
	GLfloat EndDepth;
	GLint NextRegion;
};


struct DrawParams
{
	GLuint VertexCount;
	GLuint InstanceCount;
	GLuint First;
	GLuint BaseInstance;
};

