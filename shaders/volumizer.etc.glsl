

layout(std140, binding = 0)
uniform VertexInfoBlock
{
	// Size dimensions are all powers of two
	ivec4 Mask; // (x, y, z, unused)
	ivec4 Offset; // (0, y, z, unused)
	vec4 ToUVW; // (1/size.xyz, unused)
};


layout(std140, binding = 1)
uniform VolumeInfoBlock
{
	vec4 CornerOffset; // (CornerOffset.xyz, DistanceToCorner)
	vec4 WorldSize; // (x, y, z, 1)
	vec4 WorldOrigin; // (x, y, z, 1)
};


layout(std140, binding=2)
uniform ScreenInfoBlock
{
	vec4 ScreenToClipParams; // (width/2-0.5, height/2-0.5, 1/width/2, -1/height/2)
	vec4 ScreenSize; // (width, height, 1/width, 1/height)
};


vec2 ViewToClip(vec2 Position)
{
	return (Position.xy - ScreenToClipParams.xy) * ScreenToClipParams.zw;
}


float DepthToClip(float Depth)
{
	return 1/Depth;
}
