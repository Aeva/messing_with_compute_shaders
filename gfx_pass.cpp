
#include "gfx_pass.h"


ShaderProgram CSGRenderingProgram;
GLuint ShapeParamBuffer;
GLuint InstructionBuffer;


StatusCode RenderingPass::Setup()
{
	RETURN_ON_FAIL(CSGRenderingProgram.RasterizationCompile("shaders/splat.vert", "shaders/splat.frag"));

	glGenBuffers(1, &InstructionBuffer);  
	glGenBuffers(1, &ShapeParamBuffer);

	// cheese opengl into letting us draw a full screen triangle without any data
	GLuint ScratchVAO;
	glGenVertexArrays(1, &ScratchVAO);
	glBindVertexArray(ScratchVAO);

	return StatusCode::PASS;
}


void RenderingPass::Draw()
{
	glUseProgram(CSGRenderingProgram.ProgramID);
	glDrawArraysIndirect(GL_TRIANGLE_STRIP, 0);
}
