#pragma once
#include "glue/errors.h"
#include "glue/gl_boilerplate.h"


namespace RayCastingExperiment
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

