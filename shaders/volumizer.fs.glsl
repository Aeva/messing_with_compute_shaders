prepend: shaders/volumizer.etc.glsl
prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

in vec4 WorldPosition;
in float SliceThickness;
in float Tollerance;
out vec4 Color;


float InterleavedGradientNoise()
{
    return fract(52.9829189 * fract(dot(WorldPosition.xy, vec2(0.06711056, 0.00583715)))) * 2.0 - 1.0;
}

void main()
{
	const vec3 Noise = vec3(0, 0, InterleavedGradientNoise() * SliceThickness * 16);
	const float SDF1 = HelloWorldSDF(WorldPosition.xyz + Noise);

	if (SDF1 <= Tollerance)
	{
		Color = vec4(VolumeWorldToUVW(WorldPosition.xyz + Noise), 1.0);
	}
	else
	{
		const vec3 AdjustHigher = Noise + vec3(0, 0, SDF1);
		const float SDFHigher = HelloWorldSDF(WorldPosition.xyz + AdjustHigher);
		//const float Direction = SDFLower < SDFHigher ? -1 : 1;

		if (SDFHigher <= Tollerance)
		{
			Color = vec4(VolumeWorldToUVW(WorldPosition.xyz + AdjustHigher), 1.0);
		}
		else
		{
			discard;
			/*
			const vec3 AdjustLower = Noise - vec3(0, 0, SDF1);
			const float SDFLower = HelloWorldSDF(WorldPosition.xyz + AdjustLower);

			if (SDFLower <= Tollerance)
			{
				Color = vec4(VolumeWorldToUVW(WorldPosition.xyz + AdjustLower), 1.0);
			}
			else
			{
				const float Direction = SDFLower < SDFHigher ? -1 : 1;
			}


			const vec3 Adjust2 = Noise + vec3(0, 0, SDF1);
			const float SDF3 = HelloWorldSDF(WorldPosition.xyz + Adjust2);
			if (SDF3 <= Tollerance)
			{
				Color = vec4(VolumeWorldToUVW(WorldPosition.xyz + Adjust2), 1.0);
			}
			else
			{
				discard;
			}
			*/
		}
	}

}