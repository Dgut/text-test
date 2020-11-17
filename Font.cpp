#include "Font.h"
#include "Renderer.h"
#include <iostream>
#include <glm/gtx/transform.hpp>

constexpr int ProjectionLocation = 0;
constexpr int ModelLocation = 4;
constexpr int SamplesLocation = 8;
constexpr int ColorsLocation = 14;

constexpr const char* VertexShader = R"shader(
#version 430

layout(location = 0) in vec4 position;

out vec2 polar;
flat out int id;

layout(location = 0) uniform mat4 projection;
layout(location = 4) uniform mat4 model;
layout(location = 8) uniform vec2 samples[6];

void main()
{
    vec4 pos = model * vec4(position.xy, 0., 1.);
    pos.xy += samples[gl_InstanceID];
    gl_Position = projection * pos;
    polar.xy = position.zw;
    id = gl_InstanceID;
}
)shader";

constexpr const char* SimpleShader = R"shader(
#version 430

flat in int id;

layout(location = 0) out vec4 color;

layout(location = 14) uniform vec4 colors[6];

void main()
{
    color = colors[id];
}
)shader";

constexpr const char* BezierShader = R"shader(
#version 430

in vec2 polar;
flat in int id;

layout(location = 0) out vec4 color;

layout(location = 14) uniform vec4 colors[6];

void main()
{
    float e = polar.y * 0.5 + polar.x;
    color = colors[id] * step(e * e, polar.x);
}
)shader";

#define vertexBuffer (buffers[0])
#define triangleBuffer (buffers[1])
#define fanBuffer (buffers[2])

Font::Font() :
    buffers(3)
{
    Shader vertex(GL_VERTEX_SHADER), simple(GL_FRAGMENT_SHADER), bezier(GL_FRAGMENT_SHADER);

    if (!vertex.Compile(VertexShader) ||
        !simple.Compile(SimpleShader) ||
        !bezier.Compile(BezierShader) ||
        !simpleProgram.Link(vertex, simple) ||
        !bezierProgram.Link(vertex, bezier))
        exit(-1);
}

Font::~Font()
{
}

void Font::AddContour(Glyph& glyph, float x, float y, float t, DrawParams& params)
{
    GLushort index = (GLushort)points.size();
    GLushort fan_size = (GLushort)fan.size();

    params.length = fan_size - params.start;
    if (params.length > 2)
        glyph.fans.push_back(params);
    params.start = fan_size;

    fan.push_back(index);
    fan.push_back(index + 1);

    points.push_back({ 0.f, 0.f, 0.f, 0.f });
    points.push_back({ x, y, t, 0.f });
}

void Font::AddLine(Glyph& glyph, float x, float y, float t)
{
    GLushort index = (GLushort)points.size();

    fan.push_back(index);

    points.push_back({ x, y, t, 0.f });
}

void Font::AddCurve(Glyph& glyph, float px, float py, float x, float y, float t)
{
    GLushort index = (GLushort)points.size();

    points.push_back({ px, py, 0.f, 1.f });
    points.push_back({ x, y, t, 0.f });

    fan.push_back(index + 1);

    triangles.push_back(index - 1);
    triangles.push_back(index);
    triangles.push_back(index + 1);
}

void Font::FinishGlyph(Glyph& glyph, DrawParams& params)
{
    GLushort fan_size = (GLushort)fan.size();
    params.length = fan_size - params.start;
    if (params.length > 2)
        glyph.fans.push_back(params);

    glyph.triangles.length = (GLushort)triangles.size() - glyph.triangles.start;
}

Glyph& Font::CreateGlyph(const char c, GLfloat advance, DrawParams& params)
{
    Glyph& glyph = glyphs[c];

    glyph.advance = advance;
    glyph.triangles.start = (GLushort)triangles.size();

    params.start = (GLushort)fan.size();
    params.length = 0;

    return glyph;
}

const bool Font::HasGlyph(const char c) const
{
    return glyphs.find(c) != glyphs.end();
}

void Font::FillBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec4), &points[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(GLushort), &triangles[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fanBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fan.size() * sizeof(GLushort), &fan[0], GL_STATIC_DRAW);

    points.clear();
    triangles.clear();
    fan.clear();
}

void Font::Print(float x, float y, const char* str, const float* colors, const float* samples, GLsizei count, Renderer& renderer)
{
    size_t len = strlen(str);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glVertexPointer(4, GL_FLOAT, sizeof(glm::vec4), 0);

    glUseProgram(bezierProgram);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);

    glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE , &renderer.Projection()[0][0]);
    
    glUniform4fv(ColorsLocation, count, colors);
    glUniform2fv(SamplesLocation, count, samples);

    renderer.Push();
    renderer.Multiply(glm::translate(glm::vec3(x, y, 0.f)));

    renderer.Push();

    for (size_t i = 0; i < len; i++)
    {
        if (!HasGlyph(str[i]))
            continue;

        glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, &renderer.Model()[0][0]);

        Glyph& g = glyphs[str[i]];

        if (g.triangles.length > 0)
            glDrawElementsInstanced(GL_TRIANGLES, g.triangles.length, GL_UNSIGNED_SHORT, (void*)(g.triangles.start * sizeof(GLushort)), count);

        renderer.Multiply(glm::translate(glm::vec3(g.advance, 0.f, 0.f)));
    }

    renderer.Pop();

    glUseProgram(simpleProgram);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fanBuffer);

    glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &renderer.Projection()[0][0]);
    glUniform4fv(ColorsLocation, count, colors);
    glUniform2fv(SamplesLocation, count, samples);

    renderer.Push();

    for (size_t i = 0; i < len; i++)
    {
        if (!HasGlyph(str[i]))
            continue;

        glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, &renderer.Model()[0][0]);

        Glyph& g = glyphs[str[i]];

        for (auto& f : g.fans)
            glDrawElementsInstanced(GL_TRIANGLE_FAN, f.length, GL_UNSIGNED_SHORT, (void*)(f.start * sizeof(GLushort)), count);

        renderer.Multiply(glm::translate(glm::vec3(g.advance, 0.f, 0.f)));
    }

    renderer.Pop();
    renderer.Pop();
}
