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
	vec3 Center;
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


// Group size is arbitrary.
// Each thread is a tile.
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
void main()
{
	const vec3 TileMin = vec3(vec2(gl_GlobalInvocationID.xy*TILE_SIZE), 0);
	const vec3 TileExtent = vec3(TILE_SIZE, TILE_SIZE, 1024) * 0.5;
	const vec3 TileCenter = TileMin + TileExtent;
	const uint TileID = ((gl_GlobalInvocationID.x << TILE_X_OFFSET) & TILE_X_MASK) | (gl_GlobalInvocationID.y & TILE_Y_MASK);

	int HeadChunk = -1;
	int LastChunk = -1;
	float StartDepth = 0;
	float EndDepth = -1;
	uint PathDepth = 0;

	// Generously assuming that the bounds list is sorted front to back.
	for (uint i=0; i<PositiveSpace.Count; ++i)
	{
		//const Bounds Test = PositiveSpace.Data[i];

		// HACK hard coded test data, because for some reason PositiveSpace.Data[n].Center is
		// showing the extent data, and .Extent is just 0.
		Bounds Test;
		if (i == 0)
		{
			Test.Center = vec3(250, 220, 50);
			Test.Extent = vec4(20, 20, 20, 20);
		}
		if (i == 1)
		{
			Test.Center = vec3(200, 200, 200);
			Test.Extent = vec4(100, 100, 100, 100);
		}
		if (i == 2)
		{
			Test.Center = vec3(300, 200, 200);
			Test.Extent = vec4(50, 50, 50, 50);
		}

		// This could be tightened a bit for spheres.
		const float Near = clamp(Test.Center.z - Test.Extent.z, 0, 1024);
		const float Far = clamp(Test.Center.z + Test.Extent.z, 0, 1024);
		const bool bAccepted = Far >= 0 && TestAABBOverlap(TileCenter, TileExtent, Test.Center, Test.Extent.xyz);
		if (bAccepted)
		{
			if (EndDepth < 0)
			{
				StartDepth = Near;
				EndDepth = Far;
			}
			else if (Near <= EndDepth)
			{
				StartDepth = min(StartDepth, Near);
				EndDepth = max(EndDepth, Far);
			}
			else
			{
				++PathDepth;
				const int WriteIndex = int(atomicAdd(ActiveRegions.Count, 1));
				ActiveRegions.Data[WriteIndex].TileID = TileID;
				ActiveRegions.Data[WriteIndex].StartDepth = StartDepth;
				ActiveRegions.Data[WriteIndex].EndDepth = EndDepth;
				ActiveRegions.Data[WriteIndex].NextRegion = -1;
				if (LastChunk == -1)
				{
					HeadChunk = WriteIndex;
				}
				else
				{
					ActiveRegions.Data[LastChunk].NextRegion = WriteIndex;
				}
				LastChunk = WriteIndex;

				StartDepth = Near;
				EndDepth = Far;
			}
		}
	}

	if (EndDepth > -1)
	{
		++PathDepth;
		const int WriteIndex = int(atomicAdd(ActiveRegions.Count, 1));
		ActiveRegions.Data[WriteIndex].TileID = TileID;
		ActiveRegions.Data[WriteIndex].StartDepth = StartDepth;
		ActiveRegions.Data[WriteIndex].EndDepth = EndDepth;
		ActiveRegions.Data[WriteIndex].NextRegion = -1;
		if (LastChunk == -1)
		{
			HeadChunk = WriteIndex;
		}
	}

	if (HeadChunk > -1)
	{
		const uint Lanes = TILE_SIZE*TILE_SIZE;
		const uint WriteIndex = atomicAdd(WorkItems.Count, Lanes);
		for (uint i=0; i<Lanes; ++i)
		{
			const uint LanePart = (i<<LANE_OFFSET) & LANE_MASK;
			const uint WorkPart = uint(HeadChunk) & WORK_MASK;
			WorkItems.PackedData[WriteIndex+i] = LanePart | WorkPart;
		}
		atomicMax(ActiveRegions.LongestPath, PathDepth);
	}
}
