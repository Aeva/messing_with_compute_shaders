prepend: shaders/screen.glsl
--------------------------------------------------------------------------------

layout(std140, binding = 0)
uniform SphereInfoBlock
{
	vec4 SphereParams; // (WorldOrigin.xyz, Radius)
	mat4 WorldMatrix;
};
