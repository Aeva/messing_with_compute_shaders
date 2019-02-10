--------------------------------------------------------------------------------

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

// - - - - SDF shape functions - - - -

float SphereSDF(vec3 Test, vec3 Origin, float Radius)
{
	return length(Test - Origin) - Radius;
}


float HelloWorldSDF(vec3 Test)
{
	float Solid = SphereSDF(Test, vec3(0, 0, 0), 200);
	float Cutaway1 = SphereSDF(Test, vec3(-50, -50, 100), 150);
	float Cutaway2 = SphereSDF(Test, vec3(100, 100, 100), 80);
	float Cutaway3 = SphereSDF(Test, vec3(-10, -10, -100), 100);
	return Difference(Difference(Difference(Solid, Cutaway1), Cutaway2), Cutaway3);
}
