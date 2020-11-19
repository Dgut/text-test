#include "Program.h"
#include <array>
#include <iostream>

using namespace std;

Program::Program() :
	id(glCreateProgram())
{
}

Program::~Program()
{
	glDeleteProgram(id);
}

bool Program::Link(Shader& vertex, Shader& fragment)
{
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);

	glLinkProgram(id);
    if (!CheckStatus(GL_LINK_STATUS))
        return false;

    glValidateProgram(id);
    if (!CheckStatus(GL_VALIDATE_STATUS))
        return false;

    return true;
}

void Program::PrepareLocations(initializer_list<const GLchar*> names)
{
    for (auto& n : names)
        uniforms.push_back(glGetUniformLocation(id, n));
}

GLint Program::CheckStatus(GLenum name)
{
    GLint status;
    glGetProgramiv(id, name, &status);
    if (!status)
    {
        array<GLchar, 2048> log;
        GLsizei length;
        glGetProgramInfoLog(id, log.size(), &length, log.data());
        cout << log.data() << endl;
    }
    return status;
}
