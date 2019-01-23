prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

layout(std140, binding = 0)
uniform VertexInfoBlock
{
	// Size dimensions are all powers of two
	ivec4 Mask; // (x, y, z, unused)
	ivec4 Offset; // (0, y, z, unused)
};

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};
out vec3 UVW;

void main()
{
	const ivec3 VolumeIndex = ivec3(
		(gl_VertexID & Mask.x),
		(gl_VertexID & Mask.y) >> Offset.y,
		(gl_VertexID & Mask.z) >> Offset.z);
	UVW = VolumeIndexToUVW(VolumeIndex);
	gl_Position = vec4(0, 0, 0, 1);
}