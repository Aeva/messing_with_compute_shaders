#include <math.h>
#include <iostream>
#include "glue/vector_math.h"
#include "glue/blob_builder.h"
#include "sdf_raycaster.h"

using namespace RayCastingExperiment;

ShaderPipeline DrawSphere;

Buffer SphereInfo;
Buffer ScreenInfo;


void SetupSphereInfo()
{
	const size_t TotalSize = 16;
	BlobBuilder Blob(TotalSize);
	// Sphere Origin
	Blob.Write(400.0f);
	Blob.Write(400.0f);
	Blob.Write(400.0f);
	// Sphere Radius
	Blob.Write(200.0f);
	SphereInfo.Initialize(Blob.Data(), TotalSize);
}


void SetupScreenInfo()
{
	const size_t Vec4Size = sizeof(GLfloat[4]);
	const size_t Mat4Size = sizeof(GLfloat[16]);
	const size_t TotalSize = Vec4Size + Mat4Size * 4;
	BlobBuilder Blob(TotalSize);
	// ScreenSize
	const float InvScreenWidth = 1.0/float(ScreenWidth);
	const float InvScreenHeight = 1.0/float(ScreenHeight);
	Blob.Write(float(ScreenWidth));
	Blob.Write(float(ScreenHeight));
	Blob.Write(InvScreenWidth);
	Blob.Write(InvScreenHeight);
	// WorldToView
	IdentityMatrix(*Blob.Advance<GLfloat[16]>());
	// ViewToWorld
	IdentityMatrix(*Blob.Advance<GLfloat[16]>());
	// ViewToClip
	IdentityMatrix(*Blob.Advance<GLfloat[16]>());
	// ClipToView
	IdentityMatrix(*Blob.Advance<GLfloat[16]>());
	ScreenInfo.Initialize(Blob.Data(), TotalSize);
}


StatusCode RayCastingExperiment::Setup()
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

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	return StatusCode::PASS;
}


void RayCastingExperiment::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawSphere.Activate();
	SphereInfo.Bind(GL_UNIFORM_BUFFER, 0);
	ScreenInfo.Bind(GL_UNIFORM_BUFFER, 1);
	glDrawArrays(GL_TRIANGLES, 0, 20 * 3);
}
