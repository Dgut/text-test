#pragma once

#include "Buffer.h"
#include "Program.h"
#include <glm/mat4x4.hpp>
#include <stack>

using namespace std;

class Font;

class Renderer
{
	GLsizei width;
	GLsizei height;

	Framebuffers framebuffers;
	Textures textures;
	Buffers buffers;

	Program program;

	glm::mat4 projection;
	stack<glm::mat4> model;
public:
	Renderer();
	~Renderer();

	void BeginFrame(GLsizei width, GLsizei height);
	void EndFrame();
	void Print(Font& font, float x, float y, const char* str);

	const glm::mat4& Projection() const
	{
		return projection;
	}

	const glm::mat4& Model() const
	{
		return model.top();
	}

	void Multiply(const glm::mat4& mat)
	{
		model.top() *= mat;
	}

	void Push()
	{
		model.push(model.top());
	}

	void Pop()
	{
		model.pop();
	}
};
