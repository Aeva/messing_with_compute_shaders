prepend: shaders/icosahedron.glsl
prepend: shaders/draw_sphere.etc.glsl
--------------------------------------------------------------------------------

out vec4 WorldPosition;

void main()
{
	const vec3 LocalPosition = Icosahedron[gl_VertexID] * vec3(SphereParams.w);
	WorldPosition = vec4(LocalPosition + SphereParams.xyz, 1);
	//gl_Position = ViewToClip * WorldToView * vec4(WorldPosition, 1.0);

	const vec2 ScreenUV = WorldPosition.xy * ScreenSize.zw;
	const vec2 Clip = mix(vec2(-1, -1), vec2(1, 1), ScreenUV);
	const float Depth = 1/WorldPosition.z;
	gl_Position = vec4(Clip.xy, Depth, 1.0);
}
