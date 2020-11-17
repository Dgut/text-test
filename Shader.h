#pragma once

#include <gl/glew.h>

class Shader
{
	GLuint id;

	GLint CheckStatus(GLenum name);
public:
	Shader(GLenum type);
	~Shader();

	bool Compile(const char* shader);

	operator GLuint() const { return id; }
};
