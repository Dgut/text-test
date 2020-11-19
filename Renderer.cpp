#include "Renderer.h"
#include "Font.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

constexpr const char* VertexShader = R"shader(
#version 410

layout(location = 0) in vec2 position;

out vec2 uv;

void main()
{
    gl_Position = vec4((position - 0.5) * 2., 0., 1.);
    uv = position;
}
)shader";

constexpr const char* FragmentShader = R"shader(
#version 410

in vec2 uv;

layout(location = 0) out vec4 color;

uniform sampler2D screen;
uniform float dx;

#define E 0.99
#define Kernel vec3(0.2, 0.6, 0.2)

vec3 hit(vec3 v)
{
    return step(E, mod(v * 255., 2.));
}

void main()
{
    vec2 delta = vec2(dx, 0.);
    vec3 center = texture(screen, uv).rgb;
    vec3 left = texture(screen, uv - delta).rgb;
    vec3 right = texture(screen, uv + delta).rgb;
    
    vec3 c = (hit(center) + hit(center * .0625)) * 0.5;
    vec3 l = (hit(left) + hit(left * .0625)) * 0.5;
    vec3 r = (hit(right) + hit(right * .0625)) * 0.5;

    l.rg = c.gr;
    r.gb = c.bg;

    color = vec4(dot(l, Kernel), dot(c, Kernel), dot(r, Kernel), 1.);
}
)shader";

Renderer::Renderer() :
	width(0),
	height(0),

	framebuffers(1),
	textures(1),
    buffers(1),
    projection(identity<mat4>()),
    model({identity<mat4>()})
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glClearColor(0., 0., 0., 1.);
    glEnableClientState(GL_VERTEX_ARRAY);

    float points[] =
    {
        0.f, 0.f,
        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);

    if (!vertex.Compile(VertexShader) ||
        !fragment.Compile(FragmentShader) ||
        !program.Link(vertex, fragment))
        exit(-1);

    program.PrepareLocations({"dx", "screen"});

    glProgramUniform1i(program, program[1], 0);
}

Renderer::~Renderer()
{
}

void Renderer::BeginFrame(GLsizei width, GLsizei height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);

	if ((Renderer::width != width || Renderer::height != height) && width && height)
	{
        Renderer::width = width;
        Renderer::height = height;

        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0);

        GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, buffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            cout << "Framebuffer creation failed" << endl;
            exit(-1);
        }

        //glUseProgram(program);
        glProgramUniform1f(program, program[0], 1.f / width);

        projection = glm::ortho(-width / 2., width / 2., -height / 2., height / 2.);
	}

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
}

void Renderer::EndFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width, height);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, 0);

    glUseProgram(program);

    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Renderer::Print(Font& font, float x, float y, const char* str)
{
    const float M = 0.5f;
    const float P = 1.f / 6.f;
    const float C = 1.f / 255.f;

    const float samples[] =
    {
        M - 0 * P, M - 1 * P,
        M - 1 * P, M - 4 * P,
        M - 2 * P, M - 0 * P,
        M - 3 * P, M - 3 * P,
        M - 4 * P, M - 2 * P,
        M - 5 * P, M - 5 * P,
    };

    const float colors[]
    {
        C, 0.f, 0.f, 1.f,
        C * 16.f, 0.f, 0.f, 1.f,
        0.f, C, 0.f, 1.f,
        0.f, C * 16.f, 0.f, 1.f,
        0.f, 0.f, C, 1.f,
        0.f, 0.f, C * 16.f, 1.f,
    };

    font.Print(x, y, str, colors, samples, 6, *this);
}
