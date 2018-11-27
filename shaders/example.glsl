#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_shading_language_420pack : require


layout(std140, binding = 0) uniform CSGUniformsBlock
{
	uint RegionCount;
	mat4 Projection;
};


layout(std430, binding = 1) buffer DispatchControlBlock
{
	uint VertexCount;
	uint InstanceCount;
	uint First;
  	uint BaseInstance;
} DispatchControl;


struct CSGRegion
{
	vec3 BoundsMin;
	vec3 BoundsMax;
};


layout(std430, binding = 2) buffer RegionDataBlock
{
	CSGRegion Regions[];
} RegionData;


layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
	if (gl_GlobalInvocationID.x == 0)
	{
		DispatchControl.InstanceCount = 0;
	}
	memoryBarrierBuffer();
	if (gl_GlobalInvocationID.x < RegionCount)
	{
		const CSGRegion Region = RegionData.Regions[gl_GlobalInvocationID.x];
	
		// I imagine frustums will be involved somewhere eventually
		bool bCullingPassed = Region.BoundsMin.x <= Region.BoundsMax.x;

		if (bCullingPassed)
		{
			atomicAdd(DispatchControl.InstanceCount, 1);
		}
	}
}
