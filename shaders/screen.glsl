--------------------------------------------------------------------------------

layout(std140, binding = 1)
uniform ViewInfoBlock
{
	vec4 ScreenSize; // (Width, Height, InvWidth, InvHeight)
	mat4 WorldToView;
	mat4 ViewToWorld;
	mat4 ViewToClip;
	mat4 ClipToView;
};
