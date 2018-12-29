#pragma once
#include "../errors.h"
#include "../gl_boilerplate.h"


namespace CullingPass
{
	StatusCode Setup();
	void Dispatch();
}


struct Bounds
{
	GLfloat Center[4];
	GLfloat Extent[4];
};


struct ActiveRegion
{
	GLfloat StartDepth;
	GLfloat EndDepth;
	GLint NextRegion;
};

