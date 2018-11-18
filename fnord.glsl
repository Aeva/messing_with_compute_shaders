#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require

shared uint morp;

//layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
void main()
{
	uint offset = 1 << gl_LocalInvocationIndex;
	atomicOr(morp, offset);
	groupMemoryBarrier();

	float RedChanne = float(gl_LocalInvocationIndex % 2);

	vec4 Pixel = vec4(RedChanne, 0.0, 0.0, 1.0);
	ivec2 PixelCoords = ivec2(gl_GlobalInvocationID.xy);
	imageStore(img_output, PixelCoords, Pixel);
}