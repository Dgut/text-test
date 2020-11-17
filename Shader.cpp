#include "Shader.h"
#include <array>
#include <iostream>

using namespace std;

Shader::Shader(GLenum type) :
	id(glCreateShader(type))
{
}

Shader::~Shader()
{
	glDeleteShader(id);
}

bool Shader::Compile(const char* shader)
{
	GLint len = strlen(shader);
	glShaderSource(id, 1, &shader, &len);
    glCompileShader(id);

    if (!CheckStatus(GL_COMPILE_STATUS))
        return false;
    
    return true;
}

GLint Shader::CheckStatus(GLenum name)
{
    GLint status;
    glGetShaderiv(id, name, &status);
    if (!status)
    {
        array<GLchar, 2048> log;
        GLsizei length;
        glGetShaderInfoLog(id, log.size(), &length, log.data());
        cout << log.data() << endl;
    }
    return status;
}
