#include "10_volume_setup/common.glsl"

layout(rgba32f, binding = 1) uniform image2D RenderResults;

shared uint SampleResults;


// Each group is a segment of a ray.
// Each thread is a sample within the ray.
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main()
{
	// Every pixel should have a group dispatched, so we can determine the
	// tile ID easily enough.
	const uvec2 TileXY = gl_WorkGroupID.xy % TILE_SIZE;
	const vec4 TileInfo = imageLoad(TileListHead, ivec2(TileXY));
	const int ListDepth = int(TileInfo.y);
	const vec2 ScreenXY = vec2(gl_WorkGroupID.xy);
	const uint SampleIndex = gl_LocalInvocationID.x;
	const uint SampleBit = 1 << SampleIndex;
	const uint LowerThanMe = ~(0xF << SampleIndex + 1) & 0xF;

	if (SampleIndex == 0)
	{
		SampleResults = 0;
	}
	groupMemoryBarrier();

	int WorkIndex = int(TileInfo.x);
	float Depth = 0;
	for (int ListIteration = 0; ListIteration < ListDepth && SampleResults == 0; ++ListIteration)
	{
		const float RegionNear = ActiveRegions.Data[WorkIndex].StartDepth;
		const float RegionFar = ActiveRegions.Data[WorkIndex].EndDepth;
		WorkIndex = ActiveRegions.Data[WorkIndex].NextRegion;
		for (float Offset = RegionNear; Offset <= RegionFar && SampleResults == 0; Offset += 16)
		{
			const vec3 SamplePoint = vec3(ScreenXY, Offset + SampleIndex);
			for (uint i=0; i<PositiveSpace.Count; ++i)
			{
				const Bounds Test = PositiveSpace.Data[i];
				if (TestPointInSphere(SamplePoint, Test.Center.xyz, Test.Extent.w))
				{
					atomicOr(SampleResults, SampleBit);
					Depth = SamplePoint.z;
					break;
				}
			}
			groupMemoryBarrier();
		}
	}

	if (SampleResults == 0 && SampleIndex == 0)
	{
		imageStore(RenderResults, ivec2(gl_WorkGroupID.xy), vec4(0, 0, 0, 0));
	}
	if ((SampleResults & LowerThanMe) == SampleBit)
	{
		imageStore(RenderResults, ivec2(gl_WorkGroupID.xy), vec4(Depth, 0, 0, 1));
	}
}
