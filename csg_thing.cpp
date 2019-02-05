#include <math.h>
#include <iostream>
#include "glue/vector_math.h"
#include "glue/blob_builder.h"
#include "csg_thing.h"

using namespace RenderingExperiment;

ShaderPipeline DrawSphere;

Buffer SphereInfo[4];
Buffer ScreenInfo;

GLuint TimingQuery;


void FillSphere(int Index, float X, float Y, float Z, float Radius)
{
	const size_t Vec4Size = sizeof(GLfloat[4]);
	const size_t TotalSize = Vec4Size;
	BlobBuilder Blob(TotalSize);
	// Sphere Origin
	Blob.Write(X);
	Blob.Write(Y);
	Blob.Write(Z);
	// Sphere Radius
	Blob.Write(Radius);
	SphereInfo[Index].Initialize(Blob.Data(), TotalSize);
}


void SetupSphereInfo()
{
	// negative radius indicates cutaway
	FillSphere(0, 0, 0, 0, 200);
	FillSphere(1, -50, -50, 100, -150);
	FillSphere(2, 100, 100, 100, -80);
	FillSphere(3, -10, -10, -100, -100);
}


void SetupScreenInfo()
{
	const size_t Vec4Size = sizeof(GLfloat[4]);
	const size_t Mat4Size = sizeof(GLfloat[16]);
	const size_t TotalSize = Vec4Size + Mat4Size * 2;
	BlobBuilder Blob(TotalSize);
	// ScreenSize
	const float InvScreenWidth = 1.0/float(ScreenWidth);
	const float InvScreenHeight = 1.0/float(ScreenHeight);
	Blob.Write(float(ScreenWidth));
	Blob.Write(float(ScreenHeight));
	Blob.Write(InvScreenWidth);
	Blob.Write(InvScreenHeight);
	// View
	auto ViewMatrix = Blob.Advance<GLfloat[16]>();
	auto InvViewMatrix = Blob.Advance<GLfloat[16]>();
	const float ViewX = 300;
	const float ViewY = 300;
	const float ViewZ = 300;
	TranslationMatrix(*ViewMatrix, ViewX, ViewY, ViewZ);
	TranslationMatrix(*InvViewMatrix, -ViewX, -ViewY, -ViewZ);
	ScreenInfo.Initialize(Blob.Data(), TotalSize);
}


StatusCode RenderingExperiment::Setup()
{
	RETURN_ON_FAIL(DrawSphere.Setup(
		{{GL_VERTEX_SHADER, "shaders/draw_sphere.vs.glsl"},
		 {GL_FRAGMENT_SHADER, "shaders/draw_sphere.fs.glsl"}}));

	SetupSphereInfo();
	SetupScreenInfo();

	// cheese opengl into letting us draw triangles without any data
  	GLuint vao;
  	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

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
	for (int i=0; i<4; ++i)
	{
		SphereInfo[i].Bind(GL_UNIFORM_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 20 * 3);
	}
	glEndQuery(GL_TIME_ELAPSED);
	GLuint DrawTime;
	glGetQueryObjectuiv(TimingQuery, GL_QUERY_RESULT, &DrawTime);
	std::cout << "Timings (ns): " << DrawTime << "\n";
}