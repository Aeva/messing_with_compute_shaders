out vec4 Color;
layout(binding = 0) uniform sampler2D RenderResults;
void main()
{
	vec2 UV = gl_FragCoord.xy/vec2(1024,768);
	vec4 Result = texture(RenderResults, UV);
	if (Result.a > 0)
	{
		float Depth = 1.0 - (log2(max(2, Result.x)) / 10.0);
		Color = vec4(Depth, Depth, Depth, 1);
	}
	else
	{
		Color = vec4(0, 0, 0, 1);
	}
}