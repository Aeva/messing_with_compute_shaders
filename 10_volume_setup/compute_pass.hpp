#pragma once
#include "../errors.hpp"
#include "../gl_boilerplate.hpp"


namespace CullingPass
{
	StatusCode Setup();
	void Dispatch();
}


struct ViewUniforms
{
	GLfloat WorldToView[16];
	GLfloat Projection[16];
};


struct CullingUniforms
{
	GLuint RegionCount;
	GLuint Padding[3];
};


struct CSGRegion
{
	// AABB
	GLfloat BoundsMin[4];
	GLfloat BoundsMax[4];
};


struct DrawParams
{
	GLuint VertexCount;
	GLuint InstanceCount;
	GLuint First;
	GLuint BaseInstance;
};


struct OutputVolume
{
	GLfloat ViewMin[4];
	GLfloat ViewMax[4];
	GLuint CSGID;
};
