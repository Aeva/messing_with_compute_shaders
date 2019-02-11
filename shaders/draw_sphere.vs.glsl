prepend: shaders/draw_sphere.etc.glsl
--------------------------------------------------------------------------------

const vec2 VertexData[6] = {
	vec2( 1.0,  1.0),
	vec2(-1.0,  1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0,  1.0)
};


out vec4 ViewCenter;
out vec4 ViewPosition;
out vec2 SpherePosition;


void main()
{
	SpherePosition = VertexData[gl_VertexID];
	ViewCenter = ViewMatrix * vec4(SphereParams.xyz, 1);

	if (ViewToClip.z != 0.0)
	{
		// Orthographic Rendering
		ViewPosition = vec4(vec3(SpherePosition.xy, 0.0) * vec3(abs(SphereParams.w)) + ViewCenter.xyz, 1);
		gl_Position = vec4((ViewPosition.xy + ViewToClip.xy) * ViewToClip.zw, 1.0/ViewPosition.z, 1.0);
	}
	else
	{
		/*
		// Perspective Rendering ...?
		ViewPosition = ViewMatrix * vec4(SpherePosition.xyz * vec3(abs(SphereParams.w)) + SphereParams.xyz, 1.0);
		gl_Position = PerspectiveMatrix * ViewPosition;
		*/
	}
}
