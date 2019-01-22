prepend: shaders/volumizer.etc.glsl
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
out vec4 WorldPosition;

#define TopLeft(Name) vec4(Name##Min.xy, Name##Depth, 1)
#define TopRight(Name) vec4(Name##Max.x, Name##Min.y, Name##Depth, 1)
#define BottomLeft(Name) vec4(Name##Min.x, Name##Max.y, Name##Depth, 1)
#define BottomRight(Name) vec4(Name##Max.xy, Name##Depth, 1)

void DrawQuad(
	vec2 ViewMin, vec2 ViewMax, float ViewDepth,
	vec2 ClipMin, vec2 ClipMax, float ClipDepth)
{
	WorldPosition = TopLeft(View);
	gl_Position = TopLeft(Clip);
	EmitVertex();

	WorldPosition = TopRight(View);
	gl_Position = TopRight(Clip);
	EmitVertex();

	WorldPosition = BottomLeft(View);
	gl_Position = BottomLeft(Clip);
	EmitVertex();

	WorldPosition = BottomRight(View);
	gl_Position = BottomRight(Clip);
	EmitVertex();
	EndPrimitive();
}

layout(points) in;
layout(triangle_strip, max_vertices = 16) out; // emit four quads per cell
void main()
{
	const float SDF = texture(SDFVolume, UVW[0]).r;
	if (SDF <= CornerOffset.w)
	{
		const vec3 Center = WorldOrigin.xyz - WorldSize.xyz * 0.5 + (UVW[0]) * WorldSize.xyz - 0.5;
		const vec3 ViewMin = Center - CornerOffset.xyz;
		const vec3 ViewMax = Center + CornerOffset.xyz;
		const vec2 ClipMin = ViewToClip(vec2(ViewMin.x, ViewMax.y));
		const vec2 ClipMax = ViewToClip(vec2(ViewMax.x, ViewMin.y));
		for (float i=0; i<4; ++i)
		{
			const float ViewDepth = mix(ViewMin.z, ViewMax.z, i * 0.25);
			const float ClipDepth = DepthToClip(ViewDepth);
			DrawQuad(vec2(ViewMin.x, ViewMax.y), vec2(ViewMax.x, ViewMin.y), ViewDepth, ClipMin, ClipMax, ClipDepth);
		}
	}
}
