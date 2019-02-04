--------------------------------------------------------------------------------

layout(std140, binding = 1)
uniform ViewInfoBlock
{
	vec4 ScreenSize; // (Width, Height, InvWidth, InvHeight)
	mat4 ViewMatrix;
	mat4 InvViewMatrix;
};
