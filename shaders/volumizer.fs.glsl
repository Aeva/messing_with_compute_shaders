prepend: shaders/volumizer.etc.glsl
prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
in float Tollerance;
out vec4 Color;

void main()
{
	//Color = vec4(1.0, 0.0, 0.5, 1.0);
	//Color = vec4(VolumeWorldToUVW(WorldPosition.xyz), 1.0);
	float SDF = HelloWorldSDF(WorldPosition.xyz);
	if (SDF <= Tollerance)
	{
		Color = vec4(VolumeWorldToUVW(WorldPosition.xyz), 1.0);
		//Color = vec4(vec3(abs(SDF)), 1.0);
		//Color = vec4(vec3(1/WorldPosition.z), 1);
	}
	else
	{
		discard;
	}
}