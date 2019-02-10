prepend: shaders/draw_box.etc.glsl
--------------------------------------------------------------------------------

const vec3 CubeStuff[36] = {
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5,-0.5, 0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3( 0.5,-0.5, 0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3( 0.5,-0.5, 0.5),
	vec3(-0.5,-0.5, 0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3(-0.5,-0.5, 0.5),
	vec3( 0.5,-0.5, 0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3( 0.5,-0.5, 0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3( 0.5,-0.5, 0.5)
};


out vec4 WorldPosition;


void main()
{
	vec4 LocalSpace = vec4(CubeStuff[gl_VertexID] * BoxExtent.xyz, 1.0);
	WorldPosition = WorldMatrix * LocalSpace;
	if (ViewToClip.z != 0.0)
	{
		// Orthographic Rendering
		vec4 ViewPosition = ViewMatrix * WorldPosition;
		gl_Position = vec4((ViewPosition.xy + ViewToClip.xy) * ViewToClip.zw, 1.0/ViewPosition.z, 1.0);
	}
	else
	{
		// Perspective Rendering
		gl_Position = ViewMatrix * PerspectiveMatrix * WorldPosition;
	}
}
