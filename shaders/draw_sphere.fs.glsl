prepend: shaders/draw_sphere.etc.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
out vec4 Color;

void main()
{
	//gl_FrontFacing

	vec3 RayOrigin = vec3(WorldPosition.xy, 0);
	vec3 RayNormal = vec3(WorldPosition.xy, 1000.0); // abitrary far plane


	Color = vec4(1.0, 0.0, 0.5, 1.0);
}
