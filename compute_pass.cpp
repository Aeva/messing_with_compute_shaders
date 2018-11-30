
#include "compute_pass.h"
using namespace CullingPass;


const unsigned int RegionCount = 1;
const unsigned int MaxOutputPlanes = 100;

ShaderProgram CSGCullingProgram;

UniformBuffer<ViewUniforms> ViewUBO;
UniformBuffer<CullingUniforms> CullingUBO;

ShaderStorageBuffer<CSGRegion> RegionSSBO;
ShaderStorageBuffer<DrawParams> DrawParamsSSBO;
ShaderStorageBuffer<OutputVolume> OutputVolumeSSBO;


void SetupCullingUniforms()
{
	ViewUniforms ViewSetup;
	const GLfloat ViewOrigin[3] = { 5, 5, 5 };
	const GLfloat ViewFocus[3] = { 0, 0, 0 };
	ViewMatrix(ViewSetup.WorldToView, ViewOrigin, ViewFocus);
	PerspectiveMatrix(ViewSetup.Projection);
	PrintMatrix(ViewSetup.WorldToView, "WorldToView");
	PrintMatrix(ViewSetup.Projection, "Projection");
	ViewUBO.Initialize(&ViewSetup);
	ViewUBO.AttachToBlock(CSGCullingProgram, "ViewUniformsBlock");

	CullingUniforms CullingSetup;
	CullingSetup.RegionCount = RegionCount;
	CullingUBO.Initialize(&CullingSetup);
	CullingUBO.AttachToBlock(CSGCullingProgram, "CullingUniformsBlock");
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
	RegionSSBO.Initialize(&TestData);
	RegionSSBO.AttachToBlock(CSGCullingProgram, "RegionDataBlock");
}


void SetupIndirectRenderingParams()
{
	DrawParams IndirectParams;
	IndirectParams.VertexCount = 4;
	IndirectParams.InstanceCount = 0;
	IndirectParams.First = 0;
	IndirectParams.BaseInstance = 0;
	DrawParamsSSBO.Initialize(&IndirectParams);
	DrawParamsSSBO.AttachToBlock(CSGCullingProgram, "IndirectDrawParamsBlock");
}


void SetupSliceOutput()
{
	OutputVolumeSSBO.Initialize(nullptr, MaxOutputPlanes);
	OutputVolumeSSBO.AttachToBlock(CSGCullingProgram, "OutputVolumesBlock");
}


bool CullingPass::Setup()
{
	const bool bCompileStatus = CSGCullingProgram.ComputeCompile("shaders/example.glsl");
	if (!bCompileStatus) return false;

	SetupCullingUniforms();
	SetupCullingAABBs();
	SetupSliceOutput();
	SetupIndirectRenderingParams();
	return true;
}


void CullingPass::Dispatch()
{
	glUseProgram(CSGCullingProgram.ProgramID);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	ViewUBO.BindBlock();
	CullingUBO.BindBlock();

	RegionSSBO.BindBlock();
	OutputVolumeSSBO.BindBlock();
	DrawParamsSSBO.BindBlock();

	glDispatchCompute(FAST_DIV_ROUND_UP(RegionCount, 64), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	DrawParamsSSBO.Bind(GL_DRAW_INDIRECT_BUFFER);
}
