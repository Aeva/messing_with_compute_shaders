
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

GLuint CSGProgram;
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


bool BuildCompute()
{
  std::string Source = ReadFile("fnord.glsl");
  const GLchar* SourcePtr = Source.data();
  const GLint SourceSize = Source.size();

  GLuint ComputeShader = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(ComputeShader, 1, &SourcePtr, &SourceSize);
  glCompileShader(ComputeShader);

  std::string Error = CheckCompileStatus(ComputeShader);
  if (!Error.empty())
  {
    std::cout << Error << '\n';
    return false;
  }

  CSGProgram = glCreateProgram();
  glAttachShader(CSGProgram, ComputeShader);
  glLinkProgram(CSGProgram);

  Error = CheckLinkStatus(CSGProgram);
  if (!Error.empty())
  {
    std::cout << Error << '\n';
    return false;
  }

  return true;
}


bool Setup ()
{
  return BuildCompute();
}


void Render()
{
  //glUseProgram(CSGProgram);
  //glDispatchCompute(1,0,0);
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
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  GLFWwindow* Window = glfwCreateWindow(640, 480, "meep", NULL, NULL);
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

  if (!glewGetExtension("GL_ARB_compute_shader"))
  {
    std::cout << "Compute shader extension is not available!\n";
    return 1;
  }

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
