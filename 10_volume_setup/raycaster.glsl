prepend: 10_volume_setup/common.glsl
--------------------------------------------------------------------------------
layout(rg32i, binding = 0) uniform readonly iimage2D TileListHead;
layout(rgba32f, binding = 1) uniform writeonly image2D RenderResults;


float SphereSDF(vec3 Test, vec3 Origin, float Radius)
{
	return length(Test - Origin) - Radius;
}


float Union(float lhs, float rhs)
{
	return min(lhs, rhs);
}


float Intersection(float lhs, float rhs)
{
	return max(lhs, rhs);
}


float Difference(float lhs, float rhs)
{
	return max(lhs, -rhs);
}


const vec3 CutawayOrigin = vec3(250, 180, 150);
const float CutawayRadius = 100;


shared uint ResultCount = 0;
shared float ResultDepths[16];
shared float ResultSDFs[16];


// Each group is a segment of a ray.
// Each thread is a sample within the ray.
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main()
{
	// Every pixel should have a group dispatched, so we can determine the
	// tile ID easily enough.
	const uvec2 TileXY = gl_WorkGroupID.xy / TILE_SIZE;
	const vec4 TileInfo = imageLoad(TileListHead, ivec2(TileXY));
	const int ListDepth = int(TileInfo.y);
	const vec2 ScreenXY = vec2(gl_WorkGroupID.xy);
	const uint SampleIndex = gl_LocalInvocationID.x;

	if (SampleIndex == 0)
	{
		ResultCount = 0;
	}
	groupMemoryBarrier();

	float SDF = 1024;
	int WorkIndex = int(TileInfo.x);
	for (int ListIteration = 0; ListIteration < ListDepth && ResultCount == 0; ++ListIteration)
	{
		const float RegionNear = ActiveRegions.Data[WorkIndex].StartDepth;
		const float RegionFar = ActiveRegions.Data[WorkIndex].EndDepth;
		WorkIndex = ActiveRegions.Data[WorkIndex].NextRegion;
		for (float Offset = RegionNear; Offset <= RegionFar && ResultCount == 0; Offset += 16)
		{
			const vec3 SamplePoint = vec3(ScreenXY, Offset + SampleIndex);
			for (uint i=0; i<PositiveSpace.Count; ++i)
			{
				const Bounds Test = PositiveSpace.Data[i];
				//SDF = Union(SDF, SphereSDF(SamplePoint, Test.Center.xyz, Test.Extent.w));
				SDF = Union(SDF, Difference(SphereSDF(SamplePoint, Test.Center.xyz, Test.Extent.w), SphereSDF(SamplePoint, CutawayOrigin, CutawayRadius)));
			}
			if (SDF < 0.25)
			{
				const uint WriteIndex = atomicAdd(ResultCount, 1);
				ResultDepths[WriteIndex] = SamplePoint.z;
				ResultSDFs[WriteIndex] = SDF;
			}
			groupMemoryBarrier();
		}
	}
	if (SampleIndex == 0)
	{
		if (ResultCount > 0)
		{
			float BestDepth = ResultDepths[0];
			float BestSDF = ResultSDFs[0];
			float MinDistance = abs(BestSDF);
			for (int i=1; i<ResultCount; ++i)
			{
				const float TestSDF = ResultSDFs[i];
				const float TestDistance = abs(TestSDF);
				if (TestDistance < MinDistance)
				{
					MinDistance = TestDistance;
					BestDepth = ResultDepths[i];
					BestSDF = TestSDF;
				}
			}
			imageStore(RenderResults, ivec2(gl_WorkGroupID.xy), vec4(BestDepth, BestSDF, 0, 1));
		}
		else
		{
			imageStore(RenderResults, ivec2(gl_WorkGroupID.xy), vec4(0, 0, 0, 0));
		}
	}
}
