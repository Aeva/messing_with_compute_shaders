#include "../vector_math.h"
#include "compute_pass.h"
#include <vector>
using namespace CullingPass;

namespace DataSetup
{
	ShaderProgram Program;
	ShaderStorageBlock PositiveSpaceBlock;
	ShaderStorageBlock ActiveRegionsBlock;
}
namespace RayCaster
{
	ShaderProgram Program;
	ShaderStorageBlock PositiveSpaceBlock;
	ShaderStorageBlock ActiveRegionsBlock;
}

Buffer PositiveSpaceBuffer;
Buffer ActiveRegionsBuffer;

GLuint SomeUAV;
GLuint SomeUAV2;
GLuint Sampler;


struct BlobBuilder
{
	std::vector<char> Blob;
	size_t Seek;
	BlobBuilder(size_t Size, char ClearValue='\0')
	{
		Seek = 0;
		Blob.resize(Size, ClearValue);
	}
	template<typename T>
	T* Advance()
	{
		T* Handle = reinterpret_cast<T*>(Blob.data() + Seek);
		Seek += sizeof(T);
		return Handle;
	}
	template<typename T>
	void Write(T Value)
	{
		*Advance<T>() = Value;
	}
	void* Data()
	{
		return reinterpret_cast<void*>(Blob.data());
	}
};


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

	PositiveSpaceBuffer.Initialize(Blob.Data(), TotalSize);
	DataSetup::PositiveSpaceBlock.Initialize(DataSetup::Program, "PositiveSpaceBlock");
	RayCaster::PositiveSpaceBlock.Initialize(RayCaster::Program, "PositiveSpaceBlock");
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
	ActiveRegionsBuffer.Initialize(Blob.Data(), TotalSize);
	DataSetup::ActiveRegionsBlock.Initialize(DataSetup::Program, "ActiveRegionsBlock");
	RayCaster::ActiveRegionsBlock.Initialize(RayCaster::Program, "ActiveRegionsBlock");
}


void SetupWorkItems()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &SomeUAV);
	glTextureStorage2D(SomeUAV, 1, GL_RG32I, FAST_DIV_ROUND_UP(ScreenWidth, 4), FAST_DIV_ROUND_UP(ScreenHeight, 4));

	glCreateTextures(GL_TEXTURE_2D, 1, &SomeUAV2);
	glTextureStorage2D(SomeUAV2, 1, GL_RGBA32F, ScreenWidth, ScreenHeight);

	glCreateSamplers(1, &Sampler);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(Sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(Sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}


StatusCode CullingPass::Setup()
{
	RETURN_ON_FAIL(DataSetup::Program.ComputeCompile("10_volume_setup/data_setup.glsl.built"));
	RETURN_ON_FAIL(RayCaster::Program.ComputeCompile("10_volume_setup/raycaster.glsl.built"));

	SetupPositiveSpace();
	SetupActiveRegions();
	SetupWorkItems();

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return StatusCode::PASS;
}


void CullingPass::Dispatch()
{
	glUseProgram(DataSetup::Program.ProgramID);
	DataSetup::PositiveSpaceBlock.Attach(PositiveSpaceBuffer);
	DataSetup::ActiveRegionsBlock.Attach(ActiveRegionsBuffer);
	glBindImageTexture(0, SomeUAV, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32I);
	glBindSampler(0, Sampler);
	glDispatchCompute(
		FAST_DIV_ROUND_UP(ScreenWidth, 4),
		FAST_DIV_ROUND_UP(ScreenHeight, 4),
		1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(RayCaster::Program.ProgramID);
	RayCaster::PositiveSpaceBlock.Attach(PositiveSpaceBuffer);
	RayCaster::ActiveRegionsBlock.Attach(ActiveRegionsBuffer);
	glBindImageTexture(0, SomeUAV, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32I);
	glBindImageTexture(1, SomeUAV2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindSampler(0, Sampler);
	glBindSampler(1, Sampler);
	glDispatchCompute(ScreenWidth, ScreenHeight, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
