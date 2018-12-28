#include "../vector_math.h"
#include "compute_pass.h"
#include <vector>
using namespace CullingPass;


ShaderProgram CSGCullingProgram;

ShaderStorageBuffer PositiveSpaceSSBO;
ShaderStorageBuffer ActiveRegionsSSBO;
ShaderStorageBuffer WorkItemsSSBO;


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
	Data.Extent[0] = Radius;
	Data.Extent[1] = Radius;
	Data.Extent[2] = Radius;
	Data.Extent[3] = Radius;
}


void SetupPositiveSpace()
{
	const size_t Count = 3;
	const size_t PrefixSize = sizeof(GLuint);
	const size_t ArraySize = sizeof(Bounds) * Count;
	const size_t TotalSize = PrefixSize + ArraySize;

	BlobBuilder Blob(TotalSize);
	Blob.Write<GLuint>(Count);
	FillSphere(*Blob.Advance<Bounds>(), 300, 300, 50, 20);
	FillSphere(*Blob.Advance<Bounds>(), 200, 200, 200, 100);
	FillSphere(*Blob.Advance<Bounds>(), 300, 200, 200, 50);

	PositiveSpaceSSBO.Initialize(Blob.Data(), TotalSize);
	PositiveSpaceSSBO.AttachToBlock(CSGCullingProgram, "PositiveSpaceBlock");
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
	ActiveRegionsSSBO.AttachToBlock(CSGCullingProgram, "ActiveRegionsBlock");
}


void SetupWorkItemsBlock()
{
	const int MaxCount = ScreenWidth * ScreenHeight;
	const size_t PrefixSize = sizeof(GLuint);
	const size_t ArraySize = sizeof(GLuint) * MaxCount;
	const size_t TotalSize = PrefixSize + ArraySize;

	BlobBuilder Blob(TotalSize);
	Blob.Write<GLuint>(0); // Count

	WorkItemsSSBO.Initialize(Blob.Data(), TotalSize);
	WorkItemsSSBO.AttachToBlock(CSGCullingProgram, "WorkItemsBlock");
}


StatusCode CullingPass::Setup()
{
	RETURN_ON_FAIL(CSGCullingProgram.ComputeCompile("10_volume_setup/volume_setup.glsl.built"));

	SetupPositiveSpace();
	SetupActiveRegions();
	SetupWorkItemsBlock();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glFlush();

	return StatusCode::PASS;
}


void CullingPass::Dispatch()
{
	glUseProgram(CSGCullingProgram.ProgramID);
	PositiveSpaceSSBO.BindBlock();
	ActiveRegionsSSBO.BindBlock();
	WorkItemsSSBO.BindBlock();
	glDispatchCompute(
		FAST_DIV_ROUND_UP(ScreenWidth, 4),
		FAST_DIV_ROUND_UP(ScreenHeight, 4),
		1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
