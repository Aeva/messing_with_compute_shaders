#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_shading_language_420pack : require


#define TILE_SIZE 4


struct Bounds
{
	// When the bounds describes a sphere, all four components of Extent are the radius.
	// When the bounds describes an AABB, the alpha component of Extent is zero.
	vec4 Center;
	vec4 Extent;
};
layout(std430) buffer PositiveSpaceBlock
{
	uint Count;
	Bounds Data[];
} PositiveSpace;


const uint TILE_X_OFFSET = 16;
const uint TILE_X_MASK = 0xFFFF << TILE_X_OFFSET;
const uint TILE_Y_MASK = 0xFFFF;
// TileID is packed such that the high 16 bits are the X coordinate, and the
// low 16 bits are the Y coordinate.
struct ActiveRegion
{
	uint TileID;
	float StartDepth;
	float EndDepth;
	int NextRegion;
};
layout(std430) buffer ActiveRegionsBlock
{
	uint Count;
	uint LongestPath;
	ActiveRegion Data[];
} ActiveRegions;


const uint LANE_OFFSET = 28;
const uint LANE_MASK = 0xF << LANE_OFFSET;
const uint AGE_OFFSET = 20;
const uint AGE_MASK = 0xFF << AGE_OFFSET;
const uint WORK_MASK = ~(LANE_MASK | AGE_MASK);

// Work items are packed like so:
// The highest 4 bits are the Lane index.
// The next 8 bits are the current iteration on the current work item.
// The lowest 20 bits are the index of the current work item.
layout(std430) buffer WorkItemsBlock
{
	uint Count;
	uint PackedData[];
} WorkItems;


bool TestPointInAABB(vec3 Point, vec3 BoxCenter, vec3 BoxExtent)
{
	return all(lessThanEqual(abs(BoxCenter - Point), BoxExtent));
}


bool TestPointInSphere(vec3 Point, vec3 SphereCenter, float SphereRadius)
{
	vec3 Fnord = Point - SphereCenter;
	return dot(Fnord, Fnord) <= SphereRadius * SphereRadius;
}


bool TestAABBSphereOverlap(vec3 BoxCenter, vec3 BoxExtent, vec3 SphereCenter, float SphereRadius)
{
	// Find the point in the AABB which is closest to the Sphere, and then
	// do a point-in-sphere test on it to determine overlap.
	vec3 RelativeCenter = abs(SphereCenter - BoxCenter);
	return TestPointInSphere(min(BoxExtent, RelativeCenter), RelativeCenter, SphereRadius);
}


bool TestAABBOverlap(vec3 BoxCenter1, vec3 BoxExtent1, vec3 BoxCenter2, vec3 BoxExtent2)
{
	return all(lessThanEqual(abs(BoxCenter1 - BoxCenter2), (BoxExtent1 + BoxCenter2)));
}