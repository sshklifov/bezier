#include "Common.h"
#include "FreeType.h"

#include <Renderer/Renderer.h>
#include <GLFW/glfw3.h>
#include <vector>

GLFWwindow* window;
glm::vec3* buf;
std::vector<glm::vec2> userPts;
int selectIdx = -1;
int depth = 0;
int wireMesh = 1;
int boldMesh = 0;

struct BezierCurve
{
    std::vector<glm::vec2> pts;
    float uFrom;
    float uTo;
};

// TODO
glm::vec2 Normal(glm::vec2 v)
{
    return glm::normalize(glm::vec2(-v.y, v.x));
}

static int SelectUserPts()
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 cursor(xpos, HEIGHT-ypos);

    const int threshold = 20.f;
    for (int i = 0; i < (int)userPts.size(); ++i)
    {
        if (glm::distance(userPts[i], cursor) < threshold)
        {
            return i;
        }
    }
    return -1;
}

static void MouseButtonCallback(GLFWwindow*, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        selectIdx = SelectUserPts();
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        selectIdx = -1;
    }
}

static void CursorPosCallback(GLFWwindow*, double xpos, double ypos)
{
    if (selectIdx >= 0)
    {
        xpos = glm::clamp(xpos, PADDING, WIDTH-PADDING);
        ypos = glm::clamp(ypos, PADDING, HEIGHT-PADDING);
        glm::vec2 cursor(xpos, HEIGHT-ypos);
        userPts[selectIdx] = cursor;
    }
}

static void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS &&
        userPts.size() < MAX_POINTS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = glm::clamp(xpos, PADDING, WIDTH-PADDING);
        ypos = glm::clamp(ypos, PADDING, HEIGHT-PADDING);
        
        userPts.push_back(glm::vec2(xpos, HEIGHT-ypos));
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        int idx = SelectUserPts();
        if (idx >= 0) userPts.erase(userPts.begin() + idx);
    }
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        wireMesh ^= 1;
    }
    else if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        boldMesh ^= 1;
    }
    else if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

static void ScrollCallback(GLFWwindow*, double, double yoff)
{
    if (yoff > 0)
    {
        ++depth;
    }
    else
    {
        --depth;
    }
    depth = glm::clamp(depth, 0, MAX_DEPTH);
}

void Clear(glm::vec3 color = black)
{
    int bufsiz = WIDTH*HEIGHT;
    for (int i = 0; i < bufsiz; ++i)
    {
        buf[i] = color;
    }
}

void DrawPoint(glm::vec2 p, glm::vec3 color = darkRed)
{
    const float rad = 8;
    const float edge = 10;
    assert(p.x >= edge && p.x < WIDTH-edge);
    assert(p.y >= edge && p.y < HEIGHT-edge);

    for (int yoff = -edge; yoff <= edge; ++yoff)
    {
        int index = (p.y+yoff)*WIDTH + (p.x-edge);
        for (int xoff = -edge; xoff <= edge; ++xoff)
        {
            float dst = hypot(xoff, yoff);
            float t = glm::smoothstep(rad, edge, dst);
            buf[index] = buf[index] * t + color * (1.f-t);
            glm::mix(color, buf[index], t);
            index++;
        }
    }
}

void DrawHorzLine(glm::ivec2 p, int xto, const int rad = 0, glm::vec3 color = white)
{
    if (p.x > xto) std::swap(p.x, xto);

    const int edge = rad + 2;

    for (int yoff = -edge+1; yoff < edge; ++yoff)
    {
        float t = glm::smoothstep(rad, edge, glm::abs(yoff));
        int index = (p.y+yoff)*WIDTH + p.x;
        for (int x = p.x; x <= xto; ++x)
        {
            buf[index] = glm::mix(color, buf[index], t);
            ++index;
        }
    }
}

void DrawVertLine(glm::ivec2 p, int yto, const int rad = 0, glm::vec3 color = white)
{
    if (yto < p.y) std::swap(yto, p.y);

    const int edge = rad + 2;
    assert(p.x >= rad && p.x < WIDTH-rad);

    for (int y = p.y; y <= yto; ++y)
    {
        int index = y*WIDTH + (p.x-edge+1);
        for (int xoff = -edge+1; xoff < edge; ++xoff)
        {
            float t = glm::smoothstep(rad, edge, glm::abs(xoff));
            buf[index] = glm::mix(color, buf[index], t);
            ++index;
        }
    }
}

void DrawRectangle(glm::ivec2 p1, glm::ivec2 p2, glm::vec3 color = white)
{
    int rad = boldMesh ? 1 : 0;
    if (boldMesh) color = white;

    glm::ivec2 pmin = glm::min(p1, p2);
    glm::ivec2 pmax = glm::max(p1, p2);
    DrawHorzLine(pmin, pmax.x, rad, color);
    DrawVertLine(pmin, pmax.y, rad, color);
    DrawHorzLine(pmax, pmin.x, rad, color);
    DrawVertLine(pmax, pmin.y, rad, color);
}

void DrawLine(glm::vec2 start, glm::vec2 end, glm::vec3 color = white)
{
    const float rad = 1.0;
    const float edge = 2.0;

    glm::ivec2 vmin = glm::floor(glm::min(start, end));
    glm::ivec2 vmax = glm::ceil(glm::max(start, end));

    glm::vec2 v = glm::normalize(end - start);
    glm::vec2 n(-v.y, v.x);
    float c = -glm::dot(start, n);

    if (glm::abs(n.x) < 1e-5)
        return DrawHorzLine(start, end.x, 0, color);
    float xrad = glm::abs(edge / n.x);

    for (int y = vmin.y; y <= vmax.y; ++y)
    {
        float xmid = (-n.y*y-c) / n.x;
        int xfrom = glm::clamp((int)glm::floor(xmid-xrad), vmin.x, vmax.x);
        int xto = glm::clamp((int)glm::ceil(xmid+xrad), vmin.x, vmax.x);
        int index = y*WIDTH + xfrom;
        for (int x = xfrom; x <= xto; ++x)
        {
            glm::vec2 p(x, y);
            float dst = glm::abs(glm::dot(p, n) + c);
            float t = glm::smoothstep(rad, edge, dst);
            buf[index] = glm::mix(color, buf[index], t);
            index++;
        }
    }
}

glm::vec2 BlossomBezier(const std::vector<glm::vec2>& pts, const float* ts)
{
    glm::vec2 a[MAX_POINTS];
    glm::vec2 b[MAX_POINTS];
    memcpy(a, pts.data(), pts.size() * sizeof(glm::vec2));

    glm::vec2 *cpy = a, *buf = b;
    int nts = pts.size() - 1;
    for (int i = 0; i < nts; ++i)
    {
        float t = ts[i];
        for (int j = 0; j < nts-i; ++j)
        {
            buf[j] = glm::mix(cpy[j], cpy[j+1], t);
        }
        std::swap(cpy, buf);
    }
    return cpy[0];
}

glm::vec2 BlossomBezier(const std::vector<glm::vec2>& pts, float t)
{
    float ts[MAX_POINTS];
    int nts = pts.size() - 1;
    for (int i = 0; i < nts; ++i)
    {
        ts[i] = t;
    }
    return BlossomBezier(pts, ts);
}

glm::vec2 BlossomBezier(const std::vector<glm::vec2>& pts,
    float uFrom, float uTo, int n)
{
    float ts[MAX_POINTS];
    int nts = pts.size() - 1;
    for (int i = 0; i < nts; ++i)
    {
        if (i < n) ts[i] = uTo;
        else ts[i] = uFrom;
    }
    return BlossomBezier(pts, ts);
}

void DrawCurve(const std::vector<glm::vec2>& pts, glm::vec3 color = white)
{
    float step = 0.02;
    glm::vec2 pprev = pts[0];
    for (float t = step; t < 1.0; t += step)
    {
        glm::vec2 pcur = BlossomBezier(pts, t);
        DrawLine(pprev, pcur, color);
        pprev = pcur;
    }
    DrawLine(pprev, pts.back(), color);
}

#include <cstdio>
void Subdivision(const std::vector<glm::vec2>& pts, int d = 0)
{
    glm::vec2 pmin(INFINITY), pmax(-INFINITY);
    for (const glm::vec2& p : pts)
    {
        pmin = glm::min(pmin, p);
        pmax = glm::max(pmax, p);
    }

    if (d >= depth)
    {
        if (wireMesh) DrawRectangle(pmin, pmax, gray);
        DrawLine(pts.front(), pts.back(), blue);
        return;
    }

    std::vector<glm::vec2> subdiv;
    for (int i = 0; i < (int)pts.size(); ++i)
    {
        subdiv.push_back(BlossomBezier(pts, 0.f, 0.5f, i));
    }
    Subdivision(subdiv, d+1);

    subdiv.resize(0);
    for (int i = 0; i < (int)pts.size(); ++i)
    {
        subdiv.push_back(BlossomBezier(pts, 0.5f, 1.f, i));
    }

    Subdivision(subdiv, d+1);
}

int main()
{
    window = CreateRenderer(WIDTH, HEIGHT);
    if (window == NULL)
    {
        exit(EXIT_FAILURE);
    }
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    int bufsiz = WIDTH*HEIGHT;
    buf = (glm::vec3*)malloc(bufsiz * sizeof(glm::vec3));

    InitRenderText();

    userPts.emplace_back(100, 100);
    userPts.emplace_back(200, 100);
    userPts.emplace_back(100, 200);
    userPts.emplace_back(200, 200);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        Clear();

        DrawCurve(userPts, gray);
        Subdivision(userPts);

        for (int i = 0; i < (int)userPts.size(); ++i)
        {
            DrawPoint(userPts[i]);
            RenderText(buf, i, userPts[i]);
        }

        SwapBuffers(window, buf);
    }

    free(buf);
    DestroyRenderer(window);
    return 0;
}
