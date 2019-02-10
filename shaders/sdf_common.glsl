--------------------------------------------------------------------------------

// - - - - Constants - - - -

const float DiscardThreshold = 0.00005;


// - - - - CSG operators - - - -

float Union(float lhs, float rhs)
{
	return min(lhs, rhs);
}


float Intersection(float lhs, float rhs)
{
	return max(lhs, rhs);
}


float Difference(float lhs, float rhs)
{
	return max(lhs, -rhs);
}


// - - - - SPACE transformation functions - - - -

vec3 Translate(vec3 Point, vec3 NewOrigin)
{
	return Point - NewOrigin;
}


vec2 Rotate2D(vec2 Point, float Radians)
{
	vec2 SinCos = vec2(sin(Radians), cos(Radians));
	return vec2(
		SinCos.y * Point.x + SinCos.x * Point.y,
		SinCos.y * Point.y - SinCos.x * Point.x);
}


vec3 RotateX(vec3 Point, float Radians)
{
	vec2 Rotated = Rotate2D(Point.yz, Radians);
	return vec3(Point.x, Rotated.xy);
}


vec3 RotateY(vec3 Point, float Radians)
{
	vec2 Rotated = Rotate2D(Point.xz, Radians);
	return vec3(Rotated.x, Point.y, Rotated.y);
}


vec3 RotateZ(vec3 Point, float Radians)
{
	vec2 Rotated = Rotate2D(Point.xy, Radians);
	return vec3(Rotated.xy, Point.z);
}


vec3 Scale(vec3 Point, vec3 Scale)
{
	return Point / Scale;
}


// - - - - SDF shape functions - - - -

float SphereSDF(vec3 Test, float Radius)
{
	return length(Test) - Radius;
}


float BoxSDF(vec3 Test, vec3 Extent)
{
	const vec3 EdgeDistance = abs(Test) - Extent * 0.5;
	const vec3 Positive = min(EdgeDistance, vec3(0.0));
	const vec3 Negative = max(EdgeDistance, vec3(0.0));
	const float ExteriorDistance = length(Negative);
	const float InteriorDistance = max(max(Positive.x, Positive.y), Positive.z);
	return ExteriorDistance + InteriorDistance;
}


float HelloWorldSDF(vec3 Test)
{
	float Solid = SphereSDF(Translate(Test, vec3(0, 0, 0)), 200);
	float Cutaway1 = SphereSDF(Translate(Test, vec3(-50, -50, 100)), 150);
	float Cutaway2 = SphereSDF(Translate(Test, vec3(100, 100, 100)), 80);
	float Cutaway3 = SphereSDF(Translate(Test, vec3(-10, -10, -100)), 100);
	float Cutaway4 = BoxSDF(RotateY(Test, 0.785398), vec3(100, 400, 100));
	return Difference(Difference(Difference(Difference(Solid, Cutaway1), Cutaway2), Cutaway3), Cutaway4);
}
