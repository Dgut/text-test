#pragma once

#include "Shader.h"
#include <vector>
#include <string>

class Program
{
	GLuint id;

	vector<GLint> uniforms;

	GLint CheckStatus(GLenum name);
public:
	Program();
	~Program();

	bool Link(Shader& vertex, Shader& fragment);
	void PrepareLocations(initializer_list<const GLchar*> names);

	operator GLuint() const { return id; }
	GLint operator[](int index)
	{
		return uniforms[index];
	}
};
