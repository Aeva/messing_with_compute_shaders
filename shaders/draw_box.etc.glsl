prepend: shaders/screen.glsl
--------------------------------------------------------------------------------

layout(std140, binding = 0)
uniform BoxInfoBlock
{
	vec4 BoxExtent; // (Extent.xyz, Mode) // Negative mode means subtraction.
	mat4 WorldMatrix;
	mat4 WorldRotation;
};