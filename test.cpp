
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

GLuint CSGProgram;
GLuint SplatProgram;
GLuint SomeUAV;
bool HCF = false;

const int ScreenWidth = 1024;
const int ScreenHeight = 768;

const int GroupSizeX = ScreenWidth / 8;
const int GroupSizeY = ScreenHeight / 8;
const int GroupSizeZ = 1;

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


bool BuildShaderPrograms()
{
  // compute shader
  GLuint ComputeShader;
  bool bCompiledOk = BuildShader("fnord.glsl", GL_COMPUTE_SHADER, ComputeShader);
  if (!bCompiledOk) return false;
  bool bLinkedOk = LinkProgram(&ComputeShader, 1, CSGProgram);
  if (!bLinkedOk) return false;

  // render results
  GLuint SplatShaders[2];
  bCompiledOk = BuildShader("gdi.vert", GL_VERTEX_SHADER, SplatShaders[0]);
  if (!bCompiledOk) return false;
  bCompiledOk = BuildShader("whyyyy.frag", GL_FRAGMENT_SHADER, SplatShaders[1]);
  if (!bCompiledOk) return false;
  bLinkedOk = LinkProgram(SplatShaders, 2, SplatProgram);
  if (!bLinkedOk) return false;

  return true;
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
  bool bSuccess = BuildShaderPrograms();
  if(!bSuccess) return false;

  GLuint tex_output;
  glGenTextures(1, &SomeUAV);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, SomeUAV);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindImageTexture(0, SomeUAV, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

  // cheese opengl into letting us draw a full screen triangle without any data
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  return bSuccess;
}


void Render()
{
  glUseProgram(CSGProgram);
  glDispatchCompute(GroupSizeX, GroupSizeY, GroupSizeZ);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  glUseProgram(SplatProgram);
  glDrawArrays(GL_TRIANGLES, 0, 3);
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

  if(!Setup())
  {
    std::cout << "Fatal error in application setup.\n";
    glfwTerminate();
    return 1;
  }

  while(!glfwWindowShouldClose(Window) && !HCF)
  {
    Render();
    
    glfwSwapBuffers(Window);
    glfwPollEvents();
  }   
  
  glfwDestroyWindow(Window);
  glfwTerminate();
  return 0;
}
