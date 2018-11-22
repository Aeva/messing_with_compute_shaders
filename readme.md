# OpenGL 4.2 compute shader base code

This project demonstrates how to run a basic compute shader in OpenGL
4.2.  It assumes you have OpenGL 4.2 or better, and that your drivers
support the following extensions:

 - `GL_ARB_compute_shader`
 - `GL_ARB_program_interface_query`
 - `GL_ARB_shader_storage_buffer_object`
 - `GL_ARB_shader_image_load_store`
 - `GL_ARB_gpu_shader5`

## Linux build instructions

You need python3, a c++ compiler, glfw3, and glew installed to build
this demo.

To build:
```
python3 build.py
```

To build with debugging symbols:
```
python3 build.py --debug
```

And then to run the example:
```
./hail_eris
```
