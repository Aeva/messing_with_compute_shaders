
#define TILE_SIZE 2


struct Bounds
{
	// When the bounds describes a sphere, all four components of Extent are the radius.
	// When the bounds describes an AABB, the alpha component of Extent is zero.
	vec4 Center;
	vec4 Extent;
};
layout(std430, binding = 0) buffer PositiveSpaceBlock
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
	float StartDepth;
	float EndDepth;
	int NextRegion;
};
layout(std430, binding = 1) buffer ActiveRegionsBlock
{
	uint Count;
	ActiveRegion Data[];
} ActiveRegions;


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