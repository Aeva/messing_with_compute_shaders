#include <math.h>
#include <iostream>
#include "glue/vector_math.h"
#include "glue/blob_builder.h"
#include "sdf_raycaster.h"

using namespace RayCastingExperiment;

ShaderPipeline SphereFill;
ShaderPipeline Volumizer;

const float ObjectWidth = 100;
const float VolumeSize = 16;
GLuint SDFVolume;
GLuint Sampler;

Buffer VertexInfo;
Buffer VolumeInfo;
Buffer ScreenInfo;


//GLuint TimingQueries[3];


void SetupSDFVolumes()
{
	glCreateTextures(GL_TEXTURE_3D, 1, &SDFVolume);
	glTextureStorage3D(SDFVolume, 1, GL_R32F, VolumeSize, VolumeSize, VolumeSize);

	glCreateSamplers(1, &Sampler);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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


void SetupVolumizerData()
{
	const float HalfScreenWidth = float(ScreenWidth) / 2.0;
	const float HalfScreenHeight = float(ScreenHeight) / 2.0;
	{
		const size_t TotalSize = 4 * 4 * 3;
		BlobBuilder Blob(TotalSize);
		// Masks
		Blob.Write(0x000F);
		Blob.Write(0x00F0);
		Blob.Write(0x0F00);
		Blob.Write(0);
		// Offsets
		Blob.Write(0);
		Blob.Write(4);
		Blob.Write(8);
		Blob.Write(0);
		// ToUVW
		const float ToUVW = 1.0 / VolumeSize;
		Blob.Write(ToUVW);
		Blob.Write(ToUVW);
		Blob.Write(ToUVW);
		Blob.Write(0);
		VertexInfo.Initialize(Blob.Data(), TotalSize);
	}
	{
		const size_t TotalSize = 4 * 4 * 3;
		BlobBuilder Blob(TotalSize);
		// CornerOffset
		const float CellHalfSize = ObjectWidth / VolumeSize / 2.0;
		const float CornerDistance = sqrt(CellHalfSize * CellHalfSize * 3);
		Blob.Write(CellHalfSize);
		Blob.Write(CellHalfSize);
		Blob.Write(CellHalfSize);
		Blob.Write(CornerDistance);
		// WorldSize
		Blob.Write(ObjectWidth);
		Blob.Write(ObjectWidth);
		Blob.Write(ObjectWidth);
		Blob.Write(1.0f);
		// WorldOrigin
		Blob.Write(HalfScreenWidth);
		Blob.Write(HalfScreenHeight);
		Blob.Write(400.0f);
		Blob.Write(1.0f);
		VolumeInfo.Initialize(Blob.Data(), TotalSize);
	}
	{
		const size_t TotalSize = 4 * 4 * 2;
		BlobBuilder Blob(TotalSize);
		// ScreenToClipParams
		const float OffsetX = HalfScreenWidth - 0.5;
		const float OffsetY = HalfScreenHeight - 0.5;
		const float ScaleX = 1.0 / HalfScreenWidth;
		const float ScaleY = -1.0 / HalfScreenHeight;
		Blob.Write(OffsetX);
		Blob.Write(OffsetY);
		Blob.Write(ScaleX);
		Blob.Write(ScaleY);
		// ScreenSize
		Blob.Write(float(ScreenWidth));
		Blob.Write(float(ScreenHeight));
		Blob.Write(1.0f / float(ScreenWidth));
		Blob.Write(1.0f / float(ScreenHeight));
		ScreenInfo.Initialize(Blob.Data(), TotalSize);
	}
}


StatusCode RayCastingExperiment::Setup()
{
	RETURN_ON_FAIL(SphereFill.Setup(
		{{GL_COMPUTE_SHADER, "shaders/sphere_fill.glsl"}}));

	RETURN_ON_FAIL(Volumizer.Setup(
		{{GL_VERTEX_SHADER, "shaders/volumizer.vs.glsl"},
		 {GL_GEOMETRY_SHADER, "shaders/volumizer.gs.glsl"},
		 {GL_FRAGMENT_SHADER, "shaders/volumizer.fs.glsl"}}));

	glUseProgram(0);

	SetupSDFVolumes();

	SetupVolumizerData();

	// cheese opengl into letting us draw a full screen triangle without any data
  	GLuint vao;
  	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glDisable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//glGenQueries(3, TimingQueries);

	return StatusCode::PASS;
}


void RayCastingExperiment::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Volumizer.Activate();
	glBindTextureUnit(0, SDFVolume);
	glBindSampler(0, Sampler);
	VertexInfo.Bind(GL_UNIFORM_BUFFER, 0);
	VolumeInfo.Bind(GL_UNIFORM_BUFFER, 1);
	ScreenInfo.Bind(GL_UNIFORM_BUFFER, 2);
	glDrawArrays(GL_POINTS, 0, 16 * 16 * 16);
}
