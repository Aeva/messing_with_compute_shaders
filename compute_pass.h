#pragma once
#include "demo_common.h"


namespace CullingPass
{
	bool Setup();
	void Dispatch();
}


struct CullingUniforms
{
	GLuint RegionCount;
	GLfloat Projection[16];
};


struct CSGRegion
{
	// AABB
	GLfloat BoundsMin[4];
	GLfloat BoundsMax[4];
};
