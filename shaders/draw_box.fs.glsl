prepend: shaders/draw_box.etc.glsl
prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
in vec4 WorldNormal;
out vec4 Color;


vec3 Paint(vec3 Position, vec3 Normal, float SDF)
{
	return Normal * vec3(0.5) + vec3(0.5);
}


void main()
{
	const float SDF = HelloWorldSDF(WorldPosition.xyz);
	if (SDF > DiscardThreshold)
	{
		discard;
	}
	else
	{
		const float FlipNormal = gl_FrontFacing ? 1.0 : -1.0;
		Color = vec4(Paint(WorldPosition.xyz, WorldNormal.xyz * FlipNormal, SDF), 1.0);
	}
}
