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
	GLuint ScreenWidth;
	GLuint ScreenHeight;
};


struct Bounds
{
	GLfloat Center[4];
	GLfloat Extent[4];

	// Sphere variant
	Bounds(GLfloat InCenter[3], GLfloat InRadius)
	{
		for (int i=0; i<3; ++i)
		{
			Center[i] = InCenter[i];
			Extent[i] = InRadius;
		}
		Center[3] = 0;
		Extent[3] = InRadius;
	}

	// AABB variant
	Bounds(GLfloat InCenter[3], GLfloat InExtent[3])
	{
		for (int i=0; i<3; ++i)
		{
			Center[i] = InCenter[i];
			Extent[i] = InExtent[i];
		}
		Center[3] = 0;
		Extent[3] = 0;
	}
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

