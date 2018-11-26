#version 420
#extension GL_ARB_gpu_shader5 : require


void main()
{
	const vec2 Corners[4] = {
		vec2(-1, -1),
		vec2(-1, 1),
		vec2(1, -1),
		vec2(1, 1)
	};
	gl_Position = vec4(Corners[gl_VertexID], 0, 1);
}
