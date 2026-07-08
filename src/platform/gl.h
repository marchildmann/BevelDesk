// Per-OS OpenGL header includes.
#pragma once
#if defined(__EMSCRIPTEN__)
  #include <GLES3/gl3.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl3.h>
#elif defined(_WIN32)
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include <GL/gl.h>
#else
  #include <GL/gl.h>
#endif
