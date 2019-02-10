prepend: shaders/draw_box.etc.glsl
prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
out vec4 Color;


void main()
{
	Color = vec4(gl_FrontFacing ? 0.0 : 1.0, gl_FrontFacing ? 1.0 : 0.0, 0.0, 1.0);
}
