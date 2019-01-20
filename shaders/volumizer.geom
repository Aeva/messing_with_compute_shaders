prepend: shaders/volumizer_common.glsl
--------------------------------------------------------------------------------

layout(binding = 0) uniform sampler3D SDFVolume;

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[];

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

in vec3 UVW[];

layout(points) in;
layout(triangle_strip, max_vertices = 16) out; // emit four quads per cell
void main()
{
	/*
	const float SDF = texture(SDFVolume, UVW[0]).r;
	if (SDF < CornerOffset.w)
	{
		const vec3 Center = WorldOrigin.xyz - (UVW[0]) * WorldSize.xyz;
		const vec3 CellMin = ScreenToClip(Center - CornerOffset.xyz);
		const vec3 CellMax = ScreenToClip(Center + CornerOffset.xyz);
		for (float i=0; i<4; ++i)
		{
			const float SliceZ = mix(CellMin.z, CellMax.z, i * 0.25);
			gl_Position = vec4(CellMin.xy, SliceZ, 1);
			EmitVertex();
			gl_Position = vec4(CellMax.x, CellMin.y, SliceZ, 1);
			EmitVertex();
			gl_Position = vec4(CellMin.x, CellMax.y, SliceZ, 1);
			EmitVertex();
			gl_Position = vec4(CellMax.xy, SliceZ, 1);
			EmitVertex();
			EndPrimitive();
		}
	}
	*/
	gl_Position = vec4(-1, -1, 0, 1);
	EmitVertex();
	gl_Position = vec4(1, -1, 0, 1);
	EmitVertex();
	gl_Position = vec4(-1, 1, 0, 1);
	EmitVertex();
	gl_Position = vec4(1, 1, 0, 1);
	EmitVertex();
	EndPrimitive();
}
