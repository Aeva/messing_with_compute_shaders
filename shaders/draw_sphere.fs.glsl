prepend: shaders/draw_sphere.etc.glsl
prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
out vec4 Color;


vec3 Paint(vec3 Position, vec3 Normal, float SDF)
{
	//return vec4(1.0, 0.0, 0.5, 1.0);
	return (Normal + vec3(1.0)) * vec3(0.5);
}


void main()
{
	// only works for orthographic projection
	const float DistXY = distance(WorldPosition.xy, SphereParams.xy) / SphereParams.w;
	if (DistXY > 1.0)
	{
		discard;
	}
	else
	{
		const float Dir = gl_FrontFacing ? -1.0 : 1.0;
		const float Offset = sqrt(1 - DistXY * DistXY) * SphereParams.w;
		const float NewDepth = Offset * Dir + SphereParams.z;
		const vec3 NewPosition = vec3(WorldPosition.xy, NewDepth);
		const vec3 Normal = normalize(NewPosition - SphereParams.xyz);
		const float SDF = HelloWorldSDF(NewPosition);
		if (SDF > 0.001)
		{
			discard;
		}
		else
		{
			gl_FragDepth = 1.0/NewDepth;
			Color = vec4(Paint(NewPosition, Normal, SDF), 1.0);
		}
	}
}
