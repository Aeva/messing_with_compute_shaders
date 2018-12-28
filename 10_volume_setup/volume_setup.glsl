#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_shading_language_420pack : require


#define TILE_SIZE 4
#define WAVE_SIZE = 16

layout(std140) uniform ScreenBlock
{
	uint ScreenWidth;
	uint ScreenHeight;
};


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


struct ActiveRegion
{
	float StartDepth;
	float EndDepth;
	int NextRegion;
};
layout(std430) buffer ActiveRegionsBlock
{
	uint Count;
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
	const vec3 TileCenter = vec3(vec2(gl_GlobalInvocationID.xy*TILE_SIZE) + vec2(1.5, 1.5), 512);
	const vec3 TileExtent = vec3(vec2(TILE_SIZE, TILE_SIZE) * 0.5, 512);
	
	int HeadChunk = -1;
	int LastChunk = -1;
	float StartDepth = 0;
	float EndDepth = 0;

	// Generously assuming that the bounds list is sorted front to back.
	for (uint i=0; i<PositiveSpace.Count; ++i)
	{
		const Bounds Test = PositiveSpace.Data[i];
		const bool bAccepted = TestAABBOverlap(TileCenter, TileExtent, Test.Center, Test.Extent.xyz);
		if (bAccepted)
		{
			// This could be tightened a bit for spheres.
			const float Near = max(0, Test.Center.z - Test.Extent.z);
			const float Far = min(1024, Test.Center.z + Test.Extent.z);

			if (EndDepth == 0)
			{
				StartDepth = Near;
				EndDepth = Far;
			}
			else if (Near <= EndDepth && Far > EndDepth)
			{
				EndDepth = Far;
			}
			else
			{
				const int WriteIndex = int(atomicAdd(ActiveRegions.Count, 1));
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

	if (HeadChunk > -1)
	{
		const uint Lanes = TILE_SIZE*TILE_SIZE;
		const uint WriteIndex = atomicAdd(WorkItems.Count, Lanes);
		for (uint i=0; i<Lanes; ++i)
		{
			const uint LanePart = (i<<LANE_OFFSET) & LANE_MASK;
			const uint WorkPart = uint(HeadChunk) & WORK_MASK;
			WorkItems.PackedData[WriteIndex] = LanePart | WorkPart;
		}
	}
}
