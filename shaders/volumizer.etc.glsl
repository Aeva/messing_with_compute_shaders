--------------------------------------------------------------------------------

layout(std140, binding=2)
uniform ScreenInfoBlock
{
	vec4 ScreenToClipParams; // (width/2-0.5, height/2-0.5, 1/width/2, -1/height/2)
	vec4 ScreenSize; // (width, height, 1/width, 1/height)
};


vec2 ViewToClip(vec2 Position)
{
	//return (Position.xy - ScreenToClipParams.xy) * ScreenToClipParams.zw;
	return Position * ScreenSize.zw * vec2(2.0) - vec2(1.0);
}


float DepthToClip(float Depth)
{
	return 1/Depth;
}
