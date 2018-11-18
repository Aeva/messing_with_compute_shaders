#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require

shared uint morp;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
void main()
{
	uint offset = 1 << gl_LocalInvocationIndex;
	atomicOr(morp, offset);
	groupMemoryBarrier();
}