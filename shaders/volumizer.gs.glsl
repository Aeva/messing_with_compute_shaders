prepend: shaders/volumizer.etc.glsl
prepend: shaders/sdf_common.glsl
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

/*
void DrawTestQuad()
{
	WorldPosition = vec4(0, 0, 0, 1);
	gl_Position = vec4(-1, 1, 0.5, 1);
	EmitVertex();
	WorldPosition = vec4(0, 0, 0, 1);
	gl_Position = vec4(1, 1, 0.5, 1);
	EmitVertex();
	WorldPosition = vec4(0, 0, 0, 1);
	gl_Position = vec4(-1, -1, 0.5, 1);
	EmitVertex();
	WorldPosition = vec4(0, 0, 0, 1);
	gl_Position = vec4(1, -1, 0.5, 1);
	EmitVertex();
	EndPrimitive();
}
*/

layout(points) in;
layout(triangle_strip, max_vertices = 16) out; // emit four quads per cell
void main()
{
	const vec3 CellSize = VolumeExtent.xyz * vec3(VolumeInvSize);
	const float SDF = texture(SDFVolume, UVW[0]).r;
	if (SDF <= length(CellSize))
	{
		const vec3 CellCenter = VolumeUVWToWorld(UVW[0]);
		const vec3 CellMin = CellSize * -0.5 + CellCenter;
		const vec3 CellMax = CellMin + CellSize;
		if (CellMax.z > 0)
		{
			// Only generate primitives if the entire cell is in front of the viewer.
			const vec2 ViewMin = vec2(CellMin.x, CellMax.y);
			const vec2 ViewMax = vec2(CellMax.x, CellMin.y);
			const vec2 ClipMin = ViewToClip(ViewMin);
			const vec2 ClipMax = ViewToClip(ViewMax);
			for (float i=0; i<4; ++i)
			{
				const float ViewDepth = mix(CellMin.z, CellMax.z, i * 0.25);
				const float ClipDepth = DepthToClip(ViewDepth);
				DrawQuad(ViewMin, ViewMax, ViewDepth, ClipMin, ClipMax, ClipDepth);
			}
		}
	}
}
