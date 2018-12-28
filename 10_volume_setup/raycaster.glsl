#include "10_volume_setup/common.glsl"


layout(std430) buffer NextWorkItemsBlock
{
	uint Count;
	uint PackedData[];
} NextWorkItems;


shared uint SampleResults;


// Each group is a segment of a ray.
// Each thread is a sample within the ray.
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main()
{
	const uint RayIndex = gl_WorkGroupID.x;
	const uint SampleIndex = gl_LocalInvocationID.x;
	const uint SampleBit = 1 << SampleIndex;
	const uint LowerThanMe = ~(0xF << SampleIndex) & 0xF;

	const uint PackedData = WorkItems.PackedData[RayIndex];
	const uint WorkIndex = PackedData & WORK_MASK;
	const uint Iteration = (PackedData & AGE_MASK) >> AGE_OFFSET;
	const uint LaneID = (PackedData & LANE_MASK) >> LANE_OFFSET;
	const vec2 LaneXY = vec2(LineID % TILE_SIZE, LineID / TILE_SIZE);

	const float RegionNear = ActiveRegions.Data[WorkIndex].StartDepth;
	const float RegionFar = ActiveRegions.Data[WorkIndex].EndDepth;
	const uint TileID = ActiveRegions.Data[WorkIndex].TileID;
	const float TileXY = vec2(
		(TileID & TILE_X_MASK) >> TILE_X_OFFSET,
		TileID & TILE_Y_MASK);
	const vec3 SamplePoint = vec3(
		TileXY * TILE_SIZE + LaneXY,
		RegionNear + (Iteration * 16) + SampleIndex);

	if (SampleIndex == 0)
	{
		SampleResults = 0;
	}
	groupMemoryBarrier();

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

		if (TestPointInAABB(SamplePoint, Test.Center, Test.Extent))
		{
			atomicOr(SampleResults, SampleBit);
			break;
		}
	}

	groupMemoryBarrier();
	if (SampleResults == 0 && SampleIndex == 0)
	{
		// schedule a new generation
		const uint NextIteration = Iteration + 1;
		if ((RegionNear + (NextIteration * 16)) < RegionFar)
		{
			// Continue on the current work item
			const uint WriteIndex = atomicAdd(NextWorkItems.Count, 1);
			const uint NewAge = (NextIteration << AGE_OFFSET) & AGE_MASK;
			NextWorkItems.PackedData[WriteIndex] = NewAge | PackedData;
		}
		else
		{
			const int NextRegion = ActiveRegions.Data[WorkIndex].NextRegion;
			if (NextRegion > -1)
			{
				// Move on to the next work item
				const uint WriteIndex = atomicAdd(NextWorkItems.Count, 1);
				NextWorkItems.PackedData[WriteIndex] = (PackedData & LANE_MASK) | (NextRegion & WORK_MASK);
			}
		}
	}
	else if ((SampleResults & SampleBit) && (SampleResults & LowerThanMe) == 0)
	{
		// write out this thread's depth value
		// (SamplePoint.z)
	}
}
