#include "../vector_math.h"
#include "compute_pass.h"
#include <vector>
using namespace CullingPass;


ShaderProgram TestDataSetupProgram;
ShaderProgram RayCastingProgram;

ShaderStorageBuffer PositiveSpaceSSBO;
ShaderStorageBuffer ActiveRegionsSSBO;
//ShaderStorageBuffer WorkItemsSSBO;
GLuint SomeUAV;
GLuint SomeUAV2;


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
	FillSphere(*Blob.Advance<Bounds>(), 300, 300, 50, 20);
	FillSphere(*Blob.Advance<Bounds>(), 200, 200, 200, 100);
	FillSphere(*Blob.Advance<Bounds>(), 300, 200, 200, 50);

	PositiveSpaceSSBO.Initialize(Blob.Data(), TotalSize);
	PositiveSpaceSSBO.AttachToBlock(TestDataSetupProgram, "PositiveSpaceBlock");
	PositiveSpaceSSBO.AttachToBlock(RayCastingProgram, "PositiveSpaceBlock");
}


void SetupActiveRegions()
{
	// This happens to work out to a maximum of 16 region entries per tile,
	// but this is a linked list, and most tiles shouldn't need that many...?
	const int MaxCount = ScreenWidth * ScreenHeight;
	const size_t PrefixSize = sizeof(GLuint) * 2;
	const size_t ArraySize = sizeof(ActiveRegion) * MaxCount;
	const size_t TotalSize = PrefixSize + ArraySize;

	BlobBuilder Blob(TotalSize);
	Blob.Write<GLuint>(0); // Count
	Blob.Write<GLuint>(0); // LongestPath

	ActiveRegionsSSBO.Initialize(Blob.Data(), TotalSize);
	ActiveRegionsSSBO.AttachToBlock(TestDataSetupProgram, "ActiveRegionsBlock");
	ActiveRegionsSSBO.AttachToBlock(RayCastingProgram, "ActiveRegionsBlock");
}


void SetupWorkItemsBlock()
{
	/*
	const int MaxCount = ScreenWidth * ScreenHeight;
	const size_t PrefixSize = sizeof(GLuint);
	const size_t ArraySize = sizeof(GLuint) * MaxCount;
	const size_t TotalSize = PrefixSize + ArraySize;

	BlobBuilder Blob(TotalSize);
	Blob.Write<GLuint>(0); // Count

	WorkItemsSSBO.Initialize(Blob.Data(), TotalSize);
	WorkItemsSSBO.AttachToBlock(TestDataSetupProgram, "WorkItemsBlock");
	*/
	glGenTextures(1, &SomeUAV);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, SomeUAV);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, FAST_DIV_ROUND_UP(ScreenWidth, 4), FAST_DIV_ROUND_UP(ScreenHeight, 4), 0, GL_RG_INTEGER, GL_INT, NULL);

	glGenTextures(1, &SomeUAV2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, SomeUAV2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
}


StatusCode CullingPass::Setup()
{
	RETURN_ON_FAIL(TestDataSetupProgram.ComputeCompile("10_volume_setup/data_setup.glsl.built"));
	RETURN_ON_FAIL(RayCastingProgram.ComputeCompile("10_volume_setup/raycaster.glsl.built"));

	SetupPositiveSpace();
	SetupActiveRegions();
	SetupWorkItemsBlock();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFlush();

	return StatusCode::PASS;
}


void CullingPass::Dispatch()
{
	glUseProgram(TestDataSetupProgram.ProgramID);
	PositiveSpaceSSBO.BindBlock(0);
	ActiveRegionsSSBO.BindBlock(0);
	glBindImageTexture(0, SomeUAV, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32I);
	glDispatchCompute(
		FAST_DIV_ROUND_UP(ScreenWidth, 4),
		FAST_DIV_ROUND_UP(ScreenHeight, 4),
		1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(RayCastingProgram.ProgramID);
	PositiveSpaceSSBO.BindBlock(1);
	ActiveRegionsSSBO.BindBlock(1);
	glBindImageTexture(0, SomeUAV, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32I);
	glBindImageTexture(1, SomeUAV2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glDispatchCompute(ScreenWidth, ScreenHeight, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
