prepend: shaders/icosahedron.glsl
prepend: shaders/draw_sphere.etc.glsl
--------------------------------------------------------------------------------

out vec4 WorldPosition;
out vec3 SpherePosition;


void main()
{
	SpherePosition = Icosahedron[gl_VertexID];

	// orthographic version
	const vec4 ViewCenter = ViewMatrix * WorldMatrix * vec4(SphereParams.xyz, 1);
	const vec4 ViewPosition = vec4(SpherePosition.xyz * vec3(abs(SphereParams.w)) + ViewCenter.xyz, 1);
	WorldPosition = InvViewMatrix * ViewPosition;
	const vec2 Clip = mix(vec2(-1, -1), vec2(1, 1), ViewPosition.xy * ScreenSize.zw);
	const float Depth = 1/ViewPosition.z;
	gl_Position = vec4(Clip.xy, Depth, 1.0);

	// perspective version
	/*
	const vec4 WorldOffset = WorldMatrix * vec4(SphereParams.xyz, 1);
	WorldPosition = vec4(SpherePosition.xyz * vec3(abs(SphereParams.w)) + SphereParams.xyz, 1);
	const vec4 ViewPosition = ViewMatrix * WorldPosition;
	*/
}
