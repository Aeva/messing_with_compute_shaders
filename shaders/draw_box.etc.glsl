prepend: shaders/screen.glsl
--------------------------------------------------------------------------------

layout(std140, binding = 0)
uniform BoxInfoBlock
{
	vec4 BoxExtent;
	mat4 WorldMatrix;
	mat4 InvWorldMatrix;
};
