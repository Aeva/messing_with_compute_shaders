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
	vec4 BoundsMin;
	vec4 BoundsMax;
};


layout(std430, binding = 2) buffer CSGRegionDataBlock
{
	CSGRegion Regions[];
} CSGRegionData;


layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
	const CSGRegion Region = CSGRegionData.Regions[gl_GlobalInvocationID.x];
	



	DispatchControl.VertexCount = 4;
	DispatchControl.InstanceCount = 1;
	DispatchControl.First = 0;
	DispatchControl.BaseInstance = 0;
}
