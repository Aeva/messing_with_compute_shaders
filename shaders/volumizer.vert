prepend: shaders/volumizer_common.glsl
--------------------------------------------------------------------------------

out vec3 UVW;

void main()
{
	const ivec3 Cell = ivec3(
		(gl_VertexID & Mask.x),
		(gl_VertexID & Mask.y) >> Offset.y,
		(gl_VertexID & Mask.z) >> Offset.z);
	UVW = (vec3(Cell) + 0.5) * ToUVW.xyz;
	gl_Position = vec4(0, 0, 0, 1);
}