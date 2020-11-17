#pragma once

#include "Shader.h"

class Program
{
	GLuint id;

	GLint CheckStatus(GLenum name);
public:
	Program();
	~Program();

	bool Link(Shader& vertex, Shader& fragment);

	operator GLuint() const { return id; }
};
