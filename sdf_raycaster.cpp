#include "glue/vector_math.h"
#include "glue/blob_builder.h"
#include "sdf_raycaster.h"
#include <iostream>
using namespace RayCastingExperiment;

ShaderPipeline DataSetup;
ShaderPipeline RayCaster;
ShaderPipeline SplatPass;

Buffer PositiveSpace;
Buffer ActiveRegions;

GLuint TileListHead;
GLuint RenderingResults;
GLuint Sampler;

GLuint TimingQueries[3];


void FillSphere(Bounds &Data, GLfloat X, GLfloat Y, GLfloat Z, GLfloat Radius)
{
	Data.Center[0] = X;
	Data.Center[1] = Y;
	Data.Center[2] = Z;
	Data.Center[3] = 1;
	Data.Extent[0] = Radius;
	Data.Extent[1] = Radius;
	Data.Extent[2] = Radius;
	Data.Extent[3] = Radius;
}


void SetupPositiveSpace()
{
	const size_t Count = 3;
	const size_t PrefixSize = sizeof(GLuint) * 4;
	const size_t ArraySize = sizeof(Bounds) * Count;
	const size_t TotalSize = PrefixSize + ArraySize;

	BlobBuilder Blob(TotalSize);
	Blob.Write<GLuint>(Count);
	Blob.Write<GLuint>(0xFF); // Padding to align the array correctly.
	Blob.Write<GLuint>(0xFF);
	Blob.Write<GLuint>(0xFF);
	FillSphere(*Blob.Advance<Bounds>(), 300, 250, 50, 20);
	FillSphere(*Blob.Advance<Bounds>(), 200, 200, 200, 100);
	FillSphere(*Blob.Advance<Bounds>(), 300, 200, 200, 50);

	PositiveSpace.Initialize(Blob.Data(), TotalSize);
}


void SetupActiveRegions()
{
	// This happens to work out to a maximum of 16 region entries per tile,
	// but this is a linked list, and most tiles shouldn't need that many...?
	const int MaxCount = ScreenWidth * ScreenHeight;
	const size_t PrefixSize = sizeof(GLuint);
	const size_t ArraySize = sizeof(ActiveRegion) * MaxCount;
	const size_t TotalSize = PrefixSize + ArraySize;

	BlobBuilder Blob(TotalSize);
	ActiveRegions.Initialize(Blob.Data(), TotalSize);
}


void SetupWorkItems()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &TileListHead);
	glTextureStorage2D(TileListHead, 1, GL_RG32I, FAST_DIV_ROUND_UP(ScreenWidth, 4), FAST_DIV_ROUND_UP(ScreenHeight, 4));

	glCreateTextures(GL_TEXTURE_2D, 1, &RenderingResults);
	glTextureStorage2D(RenderingResults, 1, GL_RGBA32F, ScreenWidth, ScreenHeight);

	glCreateSamplers(1, &Sampler);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(Sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}


StatusCode RayCastingExperiment::Setup()
{
	RETURN_ON_FAIL(DataSetup.Setup(
		{{GL_COMPUTE_SHADER, "shaders/data_setup.glsl"}}));

	RETURN_ON_FAIL(RayCaster.Setup(
		{{GL_COMPUTE_SHADER, "shaders/raycaster.glsl"}}));

	RETURN_ON_FAIL(SplatPass.Setup(
		{{GL_VERTEX_SHADER, "shaders/splat.vert"},
		 {GL_FRAGMENT_SHADER, "shaders/splat.frag"}}));

	SetupPositiveSpace();
	SetupActiveRegions();
	SetupWorkItems();

	// cheese opengl into letting us draw a full screen triangle without any data
  	GLuint vao;
  	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glGenQueries(3, TimingQueries);
	glUseProgram(0);
	glClearDepth(0);

	return StatusCode::PASS;
}


void RayCastingExperiment::Dispatch()
{
	glBeginQuery(GL_TIME_ELAPSED, TimingQueries[0]);
	DataSetup.Activate();
	PositiveSpace.Bind(GL_SHADER_STORAGE_BUFFER, 0);
	ActiveRegions.Bind(GL_SHADER_STORAGE_BUFFER, 1);
	glBindImageTexture(0, TileListHead, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32I);
	glBindSampler(0, Sampler);
	glDispatchCompute(
		FAST_DIV_ROUND_UP(ScreenWidth, 4),
		FAST_DIV_ROUND_UP(ScreenHeight, 4),
		1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glEndQuery(GL_TIME_ELAPSED);

	glBeginQuery(GL_TIME_ELAPSED, TimingQueries[1]);
	RayCaster.Activate();
	glBindImageTexture(0, TileListHead, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32I);
	glBindImageTexture(1, RenderingResults, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindSampler(0, Sampler);
	glBindSampler(1, Sampler);
	glDispatchCompute(ScreenWidth, ScreenHeight, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glEndQuery(GL_TIME_ELAPSED);

	glBeginQuery(GL_TIME_ELAPSED, TimingQueries[2]);
	SplatPass.Activate();
	glBindTextureUnit(0, RenderingResults);
	glBindSampler(0, Sampler);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glEndQuery(GL_TIME_ELAPSED);

	GLuint SetupTime;
	GLuint CSGTime;
	GLuint DrawTime;
	glGetQueryObjectuiv(TimingQueries[0], GL_QUERY_RESULT, &SetupTime);
	glGetQueryObjectuiv(TimingQueries[1], GL_QUERY_RESULT, &CSGTime);
	glGetQueryObjectuiv(TimingQueries[2], GL_QUERY_RESULT, &DrawTime);

	std::cout << "Timings (ns): "
		<< SetupTime << ", "
		<< CSGTime << ", "
		<< DrawTime << "\n";
}
