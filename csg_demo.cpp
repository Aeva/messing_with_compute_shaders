
#include <iostream>
#include "errors.h"
#include "gl_boilerplate.h"
#include "10_volume_setup/compute_pass.h"

#if RENDERDOC_CAPTURE_AND_QUIT
#include "renderdoc.h"
#include <dlfcn.h>
RENDERDOC_API_1_1_2 *rdoc_api = NULL;
#endif

GLFWwindow* Window;


StatusCode FindExtension(const char* ExtensionName)
{
	if (!glewGetExtension(ExtensionName))
	{
		std::cout << "Extension is not available: " << ExtensionName << "!!\n";
		return StatusCode::FAIL;
	}
	return StatusCode::PASS;
}


#define RequireExtension(Ext) RETURN_ON_FAIL(FindExtension(Ext))


void ErrorCallback(int Error, const char* Description)
{
	std::cout << "Error: " << Description << '\n';
	SetHaltAndCatchFire();
}


void DebugCallback(
	GLenum Source, 
	GLenum Type, 
	GLuint Id, 
	GLenum Severity, 
	GLsizei MessageLength, 
	const GLchar *ErrorMessage, 
	const void *UserParam)
{
    std::cout << ErrorMessage << "\n";
}


StatusCode SetupGLFW()
{
	glfwSetErrorCallback(ErrorCallback);
	if (!glfwInit())
	{
		std::cout << "glfw init failed\n";
		return StatusCode::FAIL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if DEBUG_BUILD
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	Window = glfwCreateWindow(ScreenWidth, ScreenHeight, "meep", NULL, NULL);
	if (!Window)
	{
		std::cout << "failed to create glfw window\n";
		glfwTerminate();
		return StatusCode::FAIL;
	}
	glfwMakeContextCurrent(Window);

	GLenum GlewError = glewInit();
	if (GlewError != GLEW_OK)
	{
		std::cout << "Glew failed to initialize.\n";
		glfwTerminate();
		return StatusCode::FAIL;
	}

	RequireExtension("GL_ARB_compute_shader");
	RequireExtension("GL_ARB_program_interface_query");
	RequireExtension("GL_ARB_shader_storage_buffer_object");
	RequireExtension("GL_ARB_shader_image_load_store");
	RequireExtension("GL_ARB_gpu_shader5");
	RequireExtension("GL_ARB_clip_control");
	RequireExtension("GL_ARB_direct_state_access");
	RequireExtension("GL_ARB_buffer_storage");

#if DEBUG_BUILD
	GLint ContextFlags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &ContextFlags);
	if (ContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(&DebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else
	{
		std::cout << "Debug context not available!\n";
	}
#endif

	std::cout << glGetString(GL_VERSION) << '\n';

	GLint MaxVertexSSBOs;
	glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &MaxVertexSSBOs);
	GLint MaxFragmentSSBOs;
	glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &MaxFragmentSSBOs);
	GLint MaxComputeSSBOs;
	glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &MaxComputeSSBOs);
	std::cout << "Max Vertex SSBO Blocks: " << MaxVertexSSBOs << '\n'
    	<< "Max Fragment SSBO Blocks: " << MaxFragmentSSBOs << '\n'
		<< "Max Compute SSBO Blocks: " << MaxComputeSSBOs << '\n';

#if RENDERDOC_CAPTURE_AND_QUIT
	if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
	{
    	pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    	int RenderdocStatus = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
    	if (RenderdocStatus != 1)
    	{
    		std::cout << "Could not initialize RenderDoc.\n";
    		return StatusCode::FAIL;
    	}
	}
#endif

	return StatusCode::PASS;
}


StatusCode DemoSetup ()
{
	RETURN_ON_FAIL(CullingPass::Setup());
	//RETURN_ON_FAIL(RenderingPass::Setup());

	// Near depth = 1, Far depth = 0
	glEnable(GL_DEPTH_TEST);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glClearDepth(0);
	glDepthFunc(GL_GREATER);

	glDisable(GL_CULL_FACE);
	return StatusCode::PASS;
}


void DrawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CullingPass::Dispatch();
	//RenderingPass::Draw();
    
	glfwSwapBuffers(Window);
	glfwPollEvents();
}


#define QUIT_ON_FAIL(Expr) if (Expr == StatusCode::FAIL) return 1;


int main()
{
	QUIT_ON_FAIL(SetupGLFW());
	QUIT_ON_FAIL(DemoSetup());

#if RENDERDOC_CAPTURE_AND_QUIT
	rdoc_api->StartFrameCapture(NULL, NULL);
#else
	while(!glfwWindowShouldClose(Window) && !GetHaltAndCatchFire())
#endif
	{
		DrawFrame();
	}
#if RENDERDOC_CAPTURE_AND_QUIT
	rdoc_api->EndFrameCapture(NULL, NULL);
#endif

	glfwDestroyWindow(Window);
	glfwTerminate();
	return 0;
}
