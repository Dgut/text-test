#pragma once

#include "Program.h"
#include "Buffer.h"
#include <map>
#include <glm/mat4x4.hpp>

using namespace std;

struct DrawParams
{
    GLushort start;
    GLushort length;
};

struct Glyph
{
    vector<DrawParams> fans;
    DrawParams triangles;

    GLfloat advance;
};

class Renderer;

class Font
{
    map<char, Glyph> glyphs;

    vector <glm::vec4> points;

    vector<GLushort> fan;
    vector<GLushort> triangles;

    Buffers buffers;
    
    Program simpleProgram;
    Program bezierProgram;
public:
    Font();
    ~Font();

    void AddContour(Glyph& glyph, float x, float y, float t, DrawParams& params);
    void AddLine(Glyph& glyph, float x, float y, float t);
    void AddCurve(Glyph& glyph, float px, float py, float x, float y, float t);

    Glyph& CreateGlyph(const char c, GLfloat advance, DrawParams& params);
    void FinishGlyph(Glyph& glyph, DrawParams& params);

    const bool HasGlyph(const char c) const;

    void FillBuffers();

    void Print(float x, float y, const char* str, const float* colors, const float* samples, GLsizei count, Renderer& renderer);
};
