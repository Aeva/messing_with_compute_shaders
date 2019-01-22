prepend: shaders/volumizer.etc.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
out vec4 Color;


float SphereSDF(vec3 Test, vec3 Origin, float Radius)
{
	return length(Test - Origin) - Radius;
}

void main()
{
	float SDF = SphereSDF(WorldPosition.xyz, WorldOrigin.xyz, WorldSize.x * 0.5);
	if (SDF <= 0.0)
	{
		Color = vec4(vec3(1/WorldPosition.z), 1);
	}
	else
	{
		discard;
	}
}