#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_shading_language_420pack : require


 layout(std430, binding = 0) buffer DispatchControlBlock
 {
 	uint VertexCount;
	uint InstanceCount;
	uint First;
  	uint BaseInstance;
 } DispatchControl;


layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	DispatchControl.VertexCount = 4;
	DispatchControl.InstanceCount = 1;
	DispatchControl.First = 0;
	DispatchControl.BaseInstance = 0;
}
