#pragma once

#include <gl/glew.h>
#include <vector>

#define DEFINE_GL_ARRAY_HELPER(name, gen, del)                                 \
  struct name : public std::vector<GLuint> {                                   \
    name(size_t n) : std::vector<GLuint>(n) { gen(n, data()); }                \
    ~name() { del(size(), data()); }                                           \
  };
DEFINE_GL_ARRAY_HELPER(Buffers, glGenBuffers, glDeleteBuffers)
DEFINE_GL_ARRAY_HELPER(VertexArrays, glGenVertexArrays, glDeleteVertexArrays)
DEFINE_GL_ARRAY_HELPER(Textures, glGenTextures, glDeleteTextures)
DEFINE_GL_ARRAY_HELPER(Framebuffers, glGenFramebuffers, glDeleteFramebuffers)
