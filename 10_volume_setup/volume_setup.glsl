#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_shading_language_420pack : require


layout(std140) uniform ViewUniformsBlock
{
	mat4 WorldToView;
	mat4 Projection;
};


layout(std140) uniform CullingUniformsBlock
{
	uint RegionCount;
};


struct CSGRegion
{
	vec3 BoundsMin;
	vec3 BoundsMax;
};
layout(std430) buffer RegionDataBlock
{
	CSGRegion Regions[];
};


struct OutputVolume
{
	vec4 ViewMin;
	vec4 ViewMax;
	uint RegionID;
};
layout(std430) buffer OutputVolumesBlock
{
	OutputVolume OutputVolumes[];
};


layout(std430) buffer IndirectDrawParamsBlock
{
	uint VertexCount;
	uint InstanceCount;
	uint First;
	uint BaseInstance;
} IndirectDrawParams;


layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
	if (gl_GlobalInvocationID.x == 0)
	{
		IndirectDrawParams.InstanceCount = 0;
	}
	memoryBarrierBuffer();
	if (gl_GlobalInvocationID.x < RegionCount)
	{
		const CSGRegion Region = Regions[gl_GlobalInvocationID.x];
	
		const float X1 = Region.BoundsMin.x;
		const float Y1 = Region.BoundsMin.y;
		const float Z1 = Region.BoundsMin.z;
		const float X2 = Region.BoundsMax.x;
		const float Y2 = Region.BoundsMax.y;
		const float Z2 = Region.BoundsMax.z;

		const vec4 Corners[8] = {
			vec4(X1, Y1, Z1, 1),
			vec4(X1, Y1, Z2, 1),
			vec4(X1, Y2, Z1, 1),
			vec4(X1, Y2, Z2, 1),
			vec4(X2, Y1, Z1, 1),
			vec4(X2, Y1, Z2, 1),
			vec4(X2, Y2, Z1, 1),
			vec4(X2, Y2, Z2, 1)
		};

		vec4 ViewMin = WorldToView * Corners[0];
		vec4 ViewMax = ViewMin;
		for (int i=1; i<8; ++i)
		{
			vec4 Corner = WorldToView * Corners[i];
			ViewMin = min(Corner, ViewMin);
			ViewMax = max(Corner, ViewMax);
		}

		bool bCullingPassed = ViewMax.z < 0;

		if (bCullingPassed)
		{
			uint NewPlanes = 1;
			const uint Seek = atomicAdd(IndirectDrawParams.InstanceCount, NewPlanes);
			OutputVolumes[Seek].ViewMin = ViewMin;
			OutputVolumes[Seek].ViewMax = ViewMax;
			OutputVolumes[Seek].RegionID = gl_GlobalInvocationID.x;
		}
	}
}
