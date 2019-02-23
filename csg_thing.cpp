#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include "glue/vector_math.h"
#include "glue/blob_builder.h"
#include "csg_thing.h"

using namespace RenderingExperiment;

ShaderPipeline DrawSphere;
ShaderPipeline DrawBox;

std::vector<Buffer> SphereInfo;
std::vector<Buffer> BoxInfo;
Buffer ScreenInfo;

GLuint TimingQuery;


void FillShapesFromBlob()
{
	std::ifstream InFile("shape.blob");
	InFile.seekg(0, InFile.end);
	size_t FileLength = InFile.tellg();
	InFile.seekg(0, InFile.beg);

	const size_t LayoutMetaSize = sizeof(uint32_t) * 4;
	char LayoutMeta[LayoutMetaSize];
	InFile.read(LayoutMeta, LayoutMetaSize);

	uint32_t* LayoutCursor = (uint32_t*)LayoutMeta;
	const uint32_t SphereCount = *(LayoutCursor + 0);
	const uint32_t BoxCount = *(LayoutCursor + 1);
	const uint32_t SphereSize = *(LayoutCursor + 2);
	const uint32_t BoxSize = *(LayoutCursor + 3);

	SphereInfo.resize(SphereCount);
	BoxInfo.resize(BoxCount);

	const size_t ShapeDataSize = SphereSize * SphereCount + BoxSize * BoxCount;
	void* ShapeData = malloc(ShapeDataSize);
	InFile.read((char*)ShapeData, ShapeDataSize);

	char* Cursor = (char*)ShapeData;
	for (int i=0; i<SphereCount; ++i)
	{
		SphereInfo[i].Initialize((void*)Cursor, SphereSize);
		Cursor += SphereSize;
	}
	for (int i=0; i<BoxCount; ++i)
	{
		BoxInfo[i].Initialize((void*)Cursor, BoxSize);
		Cursor += BoxSize;
	}
	free(ShapeData);
}


void SetupScreenInfo()
{
	const size_t Vec4Size = sizeof(GLfloat[4]);
	const size_t Mat4Size = sizeof(GLfloat[16]);
	const size_t TotalSize = Vec4Size * 3 + Mat4Size * 4;
	BlobBuilder Blob(TotalSize);
	// ScreenSize
	const float InvScreenWidth = 1.0/float(ScreenWidth);
	const float InvScreenHeight = 1.0/float(ScreenHeight);
	Blob.Write(float(ScreenWidth));
	Blob.Write(float(ScreenHeight));
	Blob.Write(InvScreenWidth);
	Blob.Write(InvScreenHeight);
	// View
	auto _ViewMatrix = Blob.Advance<GLfloat[16]>();
	auto InvViewMatrix = Blob.Advance<GLfloat[16]>();
	float Translation[16];
	float Rotation[16];
	TranslationMatrix(Translation, 0.0, 0.0, 400.0);
	ZRotationMatrix(Rotation, 0.0);
	MultiplyMatrices(*_ViewMatrix, Rotation, Translation);
	InvertMatrix(*InvViewMatrix, *_ViewMatrix);
	if (true)
	{
		// Orthographic Rendering
		const float ViewToClipOffsetX = 0.0;
		const float ViewToClipOffsetY = 0.0;
		const float ViewToClipScaleX = InvScreenWidth * 2.0;
		const float ViewToClipScaleY = InvScreenHeight * 2.0;
		Blob.Write(ViewToClipOffsetX);
		Blob.Write(ViewToClipOffsetY);
		Blob.Write(ViewToClipScaleX);
		Blob.Write(ViewToClipScaleY);
		const float ClipToViewOffsetX = 0.0;
		const float ClipToViewOffsetY = 0.0;
		const float ClipToViewScaleX = float(ScreenWidth) / 2.0;
		const float ClipToViewScaleY = float(ScreenWidth) / 2.0;
		Blob.Write(ClipToViewOffsetX);
		Blob.Write(ClipToViewOffsetY);
		Blob.Write(ClipToViewScaleX);
		Blob.Write(ClipToViewScaleY);
		auto Perspective = Blob.Advance<GLfloat[16]>();
		auto InvPerspective = Blob.Advance<GLfloat[16]>();
		IdentityMatrix(*Perspective);
		IdentityMatrix(*InvPerspective);
	}
	else
	{
		// Perspective Perspective
		Blob.Write(0.0f); // ViewToClip
		Blob.Write(0.0f);
		Blob.Write(0.0f);
		Blob.Write(0.0f);
		Blob.Write(0.0f); // ClipToView
		Blob.Write(0.0f);
		Blob.Write(0.0f);
		Blob.Write(0.0f);
		auto Perspective = Blob.Advance<GLfloat[16]>();
		auto InvPerspective = Blob.Advance<GLfloat[16]>();
		PerspectiveMatrix(*Perspective, 45.0, 10.0);
		InvertMatrix(*InvPerspective, *Perspective);
	}
	ScreenInfo.Initialize(Blob.Data(), TotalSize);
}


StatusCode RenderingExperiment::Setup()
{
	RETURN_ON_FAIL(DrawSphere.Setup(
		{{GL_VERTEX_SHADER, "shaders/draw_sphere.vs.glsl"},
		 {GL_FRAGMENT_SHADER, "shaders/draw_sphere.fs.glsl"}}));

	RETURN_ON_FAIL(DrawBox.Setup(
		{{GL_VERTEX_SHADER, "shaders/draw_box.vs.glsl"},
		 {GL_FRAGMENT_SHADER, "shaders/draw_box.fs.glsl"}}));

	SetupScreenInfo();
	FillShapesFromBlob();

	// cheese opengl into letting us draw triangles without any data
  	GLuint vao;
  	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1);

	glGenQueries(1, &TimingQuery);

	return StatusCode::PASS;
}


void RenderingExperiment::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBeginQuery(GL_TIME_ELAPSED, TimingQuery);
	DrawSphere.Activate();
	ScreenInfo.Bind(GL_UNIFORM_BUFFER, 1);
	for (int i=0; i<SphereInfo.size(); ++i)
	{
		SphereInfo[i].Bind(GL_UNIFORM_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	DrawBox.Activate();
	ScreenInfo.Bind(GL_UNIFORM_BUFFER, 1);
	for (int i=0; i<BoxInfo.size(); ++i)
	{
		BoxInfo[i].Bind(GL_UNIFORM_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glEndQuery(GL_TIME_ELAPSED);
	GLuint DrawTime;
	glGetQueryObjectuiv(TimingQuery, GL_QUERY_RESULT, &DrawTime);
	std::cout << "Timings (ns): " << DrawTime << "\n";
}
