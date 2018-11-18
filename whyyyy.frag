#version 420
#extension GL_ARB_gpu_shader5 : require


uniform sampler2D Whatever;

layout(location = 0) out vec4 Color;
void main ()
{
	vec2 UV = gl_FragCoord.xy/vec2(1024,768);
	Color = vec4(UV, 0, 1);
}
