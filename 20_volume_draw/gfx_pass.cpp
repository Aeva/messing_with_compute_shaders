
#include "gfx_pass.hpp"


ShaderProgram CSGRenderingProgram;
GLuint ShapeParamBuffer;
GLuint InstructionBuffer;


StatusCode RenderingPass::Setup()
{
	RETURN_ON_FAIL(CSGRenderingProgram.RasterizationCompile("20_volume_draw/volume_draw.vert.built", "20_volume_draw/volume_draw.frag.built"));

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
