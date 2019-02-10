--------------------------------------------------------------------------------

layout(std140, binding = 1)
uniform ViewInfoBlock
{
	vec4 ScreenSize; // (Width, Height, InvWidth, InvHeight)
	mat4 ViewMatrix;
	mat4 InvViewMatrix;
	vec4 ViewToClip; // (Offset.xy, Scale.xy) Non-zero values indicate orthographic rendering.
	vec4 ClipToView; // (Offset.xy, Scale.xy)
	mat4 PerspectiveMatrix;
	mat4 InvPerspectiveMatrix;
};
