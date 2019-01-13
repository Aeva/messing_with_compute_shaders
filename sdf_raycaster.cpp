#include "glue/vector_math.h"
#include "glue/blob_builder.h"
#include "sdf_raycaster.h"
#include <iostream>
using namespace RayCastingExperiment;

ShaderPipeline SphereFill;

const float VolumeSize = 16;
GLuint SDFVolume;
GLuint Sampler;

GLuint TimingQueries[3];


void SetupSDFVolumes()
{
	glCreateTextures(GL_TEXTURE_3D, 1, &SDFVolume);
	glTextureStorage3D(SDFVolume, 1, GL_R32F, VolumeSize, VolumeSize, VolumeSize);

	glCreateSamplers(1, &Sampler);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(Sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	SphereFill.Activate();
	glBindImageTexture(0, SDFVolume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(
		FAST_DIV_ROUND_UP(VolumeSize, 4),
		FAST_DIV_ROUND_UP(VolumeSize, 4),
		VolumeSize);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


StatusCode RayCastingExperiment::Setup()
{
	RETURN_ON_FAIL(SphereFill.Setup(
		{{GL_COMPUTE_SHADER, "shaders/sphere_fill.glsl"}}));

	glUseProgram(0);

	SetupSDFVolumes();

	// cheese opengl into letting us draw a full screen triangle without any data
  	GLuint vao;
  	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glGenQueries(3, TimingQueries);
	glClearDepth(0);

	return StatusCode::PASS;
}


void RayCastingExperiment::Dispatch()
{
}
