
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "demo_common.h"
#include "compute_pass.h"
#include "gfx_pass.h"

// Set true to stop the demo.
bool HCF = false;


std::string ReadFile(const char* Path)
{
  std::ifstream File;
  File.open(Path);
  std::stringstream Reader;
  Reader << File.rdbuf();
  File.close();
  return Reader.str();
}


std::string CheckCompileStatus(GLuint ObjectId)
{
  GLint StatusValue;
  glGetShaderiv(ObjectId, GL_COMPILE_STATUS, &StatusValue);
  if (StatusValue == GL_FALSE)
  {
    GLint LogLength;
    glGetShaderiv(ObjectId, GL_INFO_LOG_LENGTH, &LogLength);
    if (LogLength)
    {
      std::string ErrorLog(LogLength, 0);
      glGetShaderInfoLog(ObjectId, LogLength, NULL, (char*) ErrorLog.data());
      return ErrorLog;
    }
    else
    {
      return std::string("An unknown error occured.");
    }
  }
  return std::string();
}


std::string CheckLinkStatus(GLuint ObjectId)
{
  GLint StatusValue;
  glGetProgramiv(ObjectId, GL_LINK_STATUS, &StatusValue);
  if (!StatusValue)
  {
    GLint LogLength;
    glGetProgramiv(ObjectId, GL_INFO_LOG_LENGTH, &LogLength);
    if (LogLength)
    {
      std::string ErrorLog(LogLength, 0);
      glGetProgramInfoLog(ObjectId, LogLength, NULL, (char*) ErrorLog.data());
      return ErrorLog;
    }
    else
    {
      return std::string("An unknown error occured.");
    }
  }
  return std::string();
}


bool BuildShader(const char* Path, GLenum ShaderType, GLuint& ShaderObject)
{
  std::cout << "Building Shader: " << Path << '\n';
  std::string Source = ReadFile(Path);
  const GLchar* SourcePtr = Source.data();
  const GLint SourceSize = Source.size();

  ShaderObject = glCreateShader(ShaderType);
  glShaderSource(ShaderObject, 1, &SourcePtr, &SourceSize);
  glCompileShader(ShaderObject);

  std::string Error = CheckCompileStatus(ShaderObject);
  if (!Error.empty())
  {
    std::cout << Error << '\n';
    return false;
  }
  return true;
}


bool LinkProgram(GLuint* ShaderObjects, int ShaderCount, GLuint& ProgramObject)
{
  ProgramObject = glCreateProgram();
  for (int i=0; i<ShaderCount; ++i)
  {
    GLuint ShaderObject = ShaderObjects[i];
    glAttachShader(ProgramObject, ShaderObject);
  }
  glLinkProgram(ProgramObject);

  std::string Error = CheckLinkStatus(ProgramObject);
  if (!Error.empty())
  {
    std::cout << Error << '\n';
    return false;
  }
  return true;
}


bool ShaderProgram::ComputeCompile(const char* ComputePath)
{
  // compute shader
  GLuint ComputeShader;
  bool bCompiledOk = BuildShader(ComputePath, GL_COMPUTE_SHADER, ComputeShader);
  if (!bCompiledOk) return false;
  return LinkProgram(&ComputeShader, 1, ProgramID);
}


bool ShaderProgram::RasterizationCompile(const char* VertexPath, const char* FragmentPath)
{
  GLuint SplatShaders[2];
  bool bCompiledOk = BuildShader(VertexPath, GL_VERTEX_SHADER, SplatShaders[0]);
  if (!bCompiledOk) return false;
  bCompiledOk = BuildShader(FragmentPath, GL_FRAGMENT_SHADER, SplatShaders[1]);
  if (!bCompiledOk) return false;
  return LinkProgram(SplatShaders, 2, ProgramID);
}


bool FindExtension(const char* ExtensionName)
{
  if (!glewGetExtension(ExtensionName))
  {
    std::cout << "Extension is not available: " << ExtensionName << "!!\n";
    return false;
  }
  return true;
}


bool Setup ()
{
  if (!CullingPass::Setup()) return false;
  if (!RenderingPass::Setup()) return false;
  return true;
}


static void ErrorCallback(int Error, const char* Description)
{
  std::cout << "Error: " << Description << '\n';
  HCF = true;
}


void DebugCallback(GLenum Source, 
       GLenum Type, 
       GLuint Id, 
       GLenum Severity, 
       GLsizei MessageLength, 
       const GLchar *ErrorMessage, 
       const void *UserParam)
{
    std::cout << ErrorMessage << "\n";
}


int main()
{
  glfwSetErrorCallback(ErrorCallback);
  if (!glfwInit())
  {
    std::cout << "glfw init failed\n";
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  GLFWwindow* Window = glfwCreateWindow(ScreenWidth, ScreenHeight, "meep", NULL, NULL);
  if (!Window)
  {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(Window);

  GLenum GlewError = glewInit();
  if (GlewError != GLEW_OK)
  {
    std::cout << "Glew failed to initialize.\n";
    glfwTerminate();
    return 1;
  }

  if (!FindExtension("GL_ARB_compute_shader")) return 1;
  if (!FindExtension("GL_ARB_program_interface_query")) return 1;
  if (!FindExtension("GL_ARB_shader_storage_buffer_object")) return 1;
  if (!FindExtension("GL_ARB_shader_image_load_store")) return 1;
  if (!FindExtension("GL_ARB_gpu_shader5")) return 1;

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

  if(!Setup())
  {
    std::cout << "Fatal error in application setup.\n";
    glfwTerminate();
    return 1;
  }

  while(!glfwWindowShouldClose(Window) && !HCF)
  {
    CullingPass::Dispatch();
    RenderingPass::Draw();
    
    glfwSwapBuffers(Window);
    glfwPollEvents();
  }   
  
  glfwDestroyWindow(Window);
  glfwTerminate();
  return 0;
}
