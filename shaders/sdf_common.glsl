--------------------------------------------------------------------------------

/*
layout(std140, binding = 1)
uniform VolumeInfoBlock
{
	vec4 VolumeMin;    // (X, Y, Z, 1)
	vec4 VolumeExtent; // (Width, Height, Depth, 0)
	vec4 VolumeParams; // (VolumeSize, InverseSize, SphereRadius, unused)
};
#define VolumeSize VolumeParams.x // aka resolution
#define VolumeInvSize VolumeParams.y
#define VolumeRadius VolumeParams.z
#define VolumeMax (VolumeMin + VolumeExtent)
#define VolumeOrigin (VolumeExtent * 0.5 + VolumeMin)

vec3 VolumeIndexToUVW(ivec3 Index)
{
	return (vec3(Index) + vec3(0.5)) * vec3(VolumeInvSize);
}

vec3 VolumeUVWToIndex(vec3 UVW)
{
	return UVW * vec3(VolumeSize) - vec3(0.5);
}

vec3 VolumeUVWToWorld(vec3 UVW)
{
	return VolumeExtent.xyz * UVW + VolumeMin.xyz;
}

vec3 VolumeIndexToWorld(ivec3 Index)
{
	return VolumeUVWToWorld(VolumeIndexToUVW(Index));
}

vec3 VolumeWorldToUVW(vec3 World)
{
	return clamp((World - VolumeMin.xyz) / VolumeExtent.xyz, 0.0, 1.0);
}
*/

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
	float Solid = SphereSDF(Test, vec3(300, 300, 300), 200);
	float Cutaway1 = SphereSDF(Test, vec3(250, 250, 400), 150);
	float Cutaway2 = SphereSDF(Test, vec3(400, 400, 400), 80);
	float Cutaway3 = SphereSDF(Test, vec3(290, 290, 200), 100);
	return Difference(Difference(Difference(Solid, Cutaway1), Cutaway2), Cutaway3);
}
