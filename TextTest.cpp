#include <iostream>
#include <vector>
#include <map>
#include <ft2build.h>
#include <freetype/ftoutln.h>
#include FT_FREETYPE_H
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include "Font.h"
#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#pragma comment(lib, "opengl32.lib")

using namespace std;

const char* const FontName = "roboto.ttf";
const char* const Message = "Hello world";

void ft_error_fatal(const char* func, FT_Error error)
{
    if (error)
    {
        cout << func << " failed with code " << error << endl;
        exit(-1);
    }
}

inline float tof(long l)
{
    return l / 64.f;
}

struct DecompositionHelper
{
    Font* font;
    Glyph* glyph;
    float t;
    DrawParams params;
};

int moveTo(const FT_Vector* to, DecompositionHelper* helper)
{
    helper->font->AddContour(*helper->glyph, tof(to->x), tof(to->y), helper->t, helper->params);
    helper->t = 1.f - helper->t;

    return 0;
}

int lineTo(const FT_Vector* to, DecompositionHelper* helper)
{
    helper->font->AddLine(*helper->glyph, tof(to->x), tof(to->y), helper->t);
    helper->t = 1.f - helper->t;

    return 0;
}

int conicTo(const FT_Vector* control, const FT_Vector* to, DecompositionHelper* helper)
{
    helper->font->AddCurve(*helper->glyph, tof(control->x), tof(control->y), tof(to->x), tof(to->y), helper->t);
    helper->t = 1.f - helper->t;

    return 0;
}

int cubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, DecompositionHelper* helper)
{
    cout << "Unfortunately, cubic curves are not supported by the algorithm :(" << endl;
    exit(-1);

    return -1;
}

float scale = 4.f;
glm::vec2 offset = glm::zero<glm::vec2>();
glm::vec2 old = glm::zero<glm::vec2>();
bool dragging = false;

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (dragging)
        {
            offset.x += (xpos - old.x) / scale;
            offset.y -= (ypos - old.y) / scale;

            old.x = xpos;
            old.y = ypos;
        }
        else
        {
            dragging = true;
            old.x = xpos;
            old.y = ypos;
        }
    }
    else
        dragging = false;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scale *= pow(1.1, yoffset);
}

void showFPS(GLFWwindow* window)
{
    static double last = 0.;
    double current = glfwGetTime();
    double delta = current - last;

    char buffer[64];
    int ret = snprintf(buffer, sizeof buffer, "%f", 1. / delta);

    glfwSetWindowTitle(window, buffer);

    last = current;
}

void Load(Font& font, const char* filename, const char* str)
{
    FT_Library  library;
    FT_Face     face;
    FT_Error    error;

    error = FT_Init_FreeType(&library);
    ft_error_fatal("FT_Init_FreeType", error);

    error = FT_New_Face(library, filename, 0, &face);
    ft_error_fatal("FT_New_Face", error);

    size_t len = strlen(str);

    for (size_t i = 0; i < len; i++)
    {
        if (font.HasGlyph(str[i]))
            continue;

        error = FT_Load_Char(face, str[i], FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
        ft_error_fatal("FT_Load_Char", error);

        FT_Outline& outline = face->glyph->outline;

        if (face->glyph->format != ft_glyph_format_outline)
            std::cout << "not an outline font\n";

        DecompositionHelper helper;

        Glyph& glyph = font.CreateGlyph(str[i], tof(face->glyph->advance.x), helper.params);

        FT_Outline_Funcs funcs;
        funcs.move_to = (FT_Outline_MoveTo_Func)&moveTo;
        funcs.line_to = (FT_Outline_LineTo_Func)&lineTo;
        funcs.conic_to = (FT_Outline_ConicTo_Func)&conicTo;
        funcs.cubic_to = (FT_Outline_CubicTo_Func)&cubicTo;
        funcs.delta = 0;
        funcs.shift = 0;

        helper.font = &font;
        helper.glyph = &glyph;
        helper.t = 0.f;

        FT_Error error = FT_Outline_Decompose(&outline, &funcs, &helper);
        ft_error_fatal("FT_Outline_Decompose", error);

        font.FinishGlyph(glyph, helper.params);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    font.FillBuffers();
}

int main()
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(800, 600, Message, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    glewInit();

    Renderer renderer;

    Font font;
    Load(font, FontName, Message);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        renderer.BeginFrame(width, height);

        renderer.Push();

        renderer.Multiply(glm::scale(glm::vec3(scale, scale, 1.f)));
        renderer.Multiply(glm::translate(glm::vec3(offset.x, offset.y, 0.f)));

        renderer.Print(font, -80, -10, Message);

        renderer.Pop();

        renderer.EndFrame();

        glfwSwapBuffers(window);

        glfwPollEvents();

        showFPS(window);
    }

    glfwTerminate();
}
