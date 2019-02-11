prepend: shaders/draw_box.etc.glsl
--------------------------------------------------------------------------------

const vec3 CubeStuff[36] = {
	// -X
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5,-0.5, 0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3(-0.5, 0.5,-0.5),

	// X
	vec3( 0.5, 0.5, 0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3( 0.5,-0.5, 0.5),

	// -Y
	vec3( 0.5,-0.5, 0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3( 0.5,-0.5, 0.5),
	vec3(-0.5,-0.5, 0.5),
	vec3(-0.5,-0.5,-0.5),

	// Y
	vec3( 0.5, 0.5, 0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3(-0.5, 0.5, 0.5),

	// -Z
	vec3( 0.5, 0.5,-0.5),
	vec3(-0.5,-0.5,-0.5),
	vec3(-0.5, 0.5,-0.5),
	vec3( 0.5, 0.5,-0.5),
	vec3( 0.5,-0.5,-0.5),
	vec3(-0.5,-0.5,-0.5),

	// Z
	vec3(-0.5, 0.5, 0.5),
	vec3(-0.5,-0.5, 0.5),
	vec3( 0.5,-0.5, 0.5),
	vec3( 0.5, 0.5, 0.5),
	vec3(-0.5, 0.5, 0.5),
	vec3( 0.5,-0.5, 0.5)
};


out vec4 WorldPosition;
out vec4 WorldNormal;


void main()
{
	const vec4 LocalSpace = vec4(CubeStuff[gl_VertexID] * BoxExtent.xyz, 1.0);
	const int Face = gl_VertexID / 6;
	const int Axis = Face / 3;
	const float Sign = float((Face % 2) * 2 - 1);
	const vec3 LocalNormal = vec3(Axis == 0, Axis == 1, Axis == 2) * Sign;
	WorldPosition = WorldMatrix * LocalSpace;
	WorldNormal = WorldRotation * vec4(LocalNormal, 1.0);
	WorldNormal = vec4(normalize(WorldNormal.xyz / WorldNormal.w), 1.0);
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
