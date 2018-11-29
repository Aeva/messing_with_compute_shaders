
#include "compute_pass.h"
using namespace CullingPass;


const unsigned int RegionCount = 1;

ShaderProgram CSGCullingProgram;
GLuint RegionBuffer;
GLuint ViewUniformBuffer;
GLuint CullingUniformBuffer;
GLuint IndirectParamsBuffer;


void SetupCullingUniforms()
{
	{
		ViewUniforms Uniforms;
		const GLfloat ViewOrigin[3] = { 5, 5, 5 };
		const GLfloat ViewFocus[3] = { 0, 0, 0 };
		ViewMatrix(Uniforms.WorldToView, ViewOrigin, ViewFocus);
		PerspectiveMatrix(Uniforms.Projection);
		PrintMatrix(Uniforms.WorldToView, "WorldToView");
		PrintMatrix(Uniforms.Projection, "Projection");
		glGenBuffers(1, &ViewUniformBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, ViewUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewUniforms), &Uniforms, GL_STATIC_DRAW);
	}
	{
		CullingUniforms Uniforms;
		Uniforms.RegionCount = RegionCount;
		glGenBuffers(1, &CullingUniformBuffer);
		glBindBuffer(GL_UNIFORM_BUFFER, CullingUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CullingUniforms), &Uniforms, GL_STATIC_DRAW);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void SetupCullingAABBs()
{
	CSGRegion TestData;
	TestData.BoundsMin[0] = -1;
	TestData.BoundsMin[1] = -1;
	TestData.BoundsMin[2] = -1;
	TestData.BoundsMax[0] = 1;
	TestData.BoundsMax[1] = 1;
	TestData.BoundsMax[2] = 1;
 	glGenBuffers(1, &RegionBuffer);
 	glBindBuffer(GL_SHADER_STORAGE_BUFFER, RegionBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CSGRegion), &TestData, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


void SetupIndirectRenderingParams()
{
	DrawArraysIndirectCommand IndirectParams;
	IndirectParams.VertexCount = 4;
	IndirectParams.InstanceCount = 0;
	IndirectParams.First = 0;
	IndirectParams.BaseInstance = 0;
	glGenBuffers(1, &IndirectParamsBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectParamsBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawArraysIndirectCommand), &IndirectParams, GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}


bool CullingPass::Setup()
{
	SetupCullingUniforms();
	SetupCullingAABBs();
	SetupIndirectRenderingParams();
	
	return CSGCullingProgram.ComputeCompile("shaders/example.glsl");
}


void CullingPass::Dispatch()
{
	glUseProgram(CSGCullingProgram.ProgramID);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ViewUniformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, CullingUniformBuffer);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, IndirectParamsBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, RegionBuffer);
	glDispatchCompute(FAST_DIV_ROUND_UP(RegionCount, 64), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, IndirectParamsBuffer);
}
