prepend: shaders/sdf_common.glsl
--------------------------------------------------------------------------------

layout(r32f, binding = 0) uniform writeonly restrict image3D SDFVolume;


// Each group is arbitrary.
// Each thread is a voxel.
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
void main()
{
	const ivec3 VolumeIndex = ivec3(gl_GlobalInvocationID.zyx);
	const vec3 SampleAt = VolumeIndexToWorld(VolumeIndex);
	const float SDF = SphereSDF(SampleAt, VolumeOrigin.xyz, VolumeRadius);
	imageStore(SDFVolume, VolumeIndex, vec4(SDF, 0, 0, 0));
}
