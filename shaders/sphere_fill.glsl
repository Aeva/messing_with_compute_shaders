--------------------------------------------------------------------------------
layout(r32f, binding = 0) uniform writeonly restrict image3D SDFVolume;
#define VolumeSize 16
const vec3 WorldMin = vec3(10, 10, 10);
const float WorldExtent = 100;
const vec3 SphereCenter = WorldExtent * 0.5 + WorldMin;
const float SphereRadius = 46;


float SphereSDF(vec3 Test, vec3 Origin, float Radius)
{
	return length(Test - Origin) - Radius;
}


// Each group is arbitrary.
// Each thread is a voxel.
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
void main()
{
	const vec3 UVW = (vec3(gl_GlobalInvocationID.zyx) + vec3(0.5)) / vec3(VolumeSize);
	const vec3 SampleAt = WorldExtent * UVW + WorldMin;

	const float SDF = SphereSDF(SampleAt, SphereCenter, SphereRadius);

	imageStore(SDFVolume, ivec3(gl_GlobalInvocationID.zyx), vec4(SDF, 0, 0, 0));
}
