
#include "gfx_pass.h"


ShaderProgram CSGRenderingProgram;
GLuint ShapeParamBuffer;
GLuint InstructionBuffer;


bool RenderingPass::Setup()
{
	glGenBuffers(1, &InstructionBuffer);  
	glGenBuffers(1, &ShapeParamBuffer);

	// cheese opengl into letting us draw a full screen triangle without any data
	GLuint ScratchVAO;
	glGenVertexArrays(1, &ScratchVAO);
	glBindVertexArray(ScratchVAO);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	return CSGRenderingProgram.RasterizationCompile("shaders/splat.vert", "shaders/splat.frag");
}


void RenderingPass::Draw()
{
	glUseProgram(CSGRenderingProgram.ProgramID);
	glDrawArraysIndirect(GL_TRIANGLE_STRIP, 0);
}
