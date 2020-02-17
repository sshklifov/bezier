#include "Common.h"
#include "FreeType.h"

#include <Renderer/Renderer.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <vector>

GLFWwindow* window;
glm::vec3* buf;
std::vector<glm::vec2> userPts;
glm::vec2* selectPtr = NULL;
int depth = 0;
int wireMesh = 1;
int boldMesh = 0;
int drawShadow = 1;

glm::vec2 a(300, 300);
glm::vec2 b(600, 500);
int intersectMode = 0;
int maxSteps = 1;

struct BezierCurve {
    std::vector<glm::vec2> pts;
    float uFrom;
    float uTo;
};

static glm::vec2* SelectPoints()
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 cursor(xpos, HEIGHT - ypos);

    const int threshold = 20.f;
    for (int i = 0; i < (int)userPts.size(); ++i) {
        if (glm::distance(userPts[i], cursor) < threshold)
            return userPts.data() + i;
    }
    if (intersectMode) {
        if (glm::distance(a, cursor) < threshold)
            return &a;
        if (glm::distance(b, cursor) < threshold)
            return &b;
    }

    return NULL;
}

static void MouseButtonCallback(GLFWwindow*, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        selectPtr = SelectPoints();
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        selectPtr = NULL;
    }
}

static void CursorPosCallback(GLFWwindow*, double xpos, double ypos)
{
    if (selectPtr) {
        xpos = glm::clamp(xpos, PADDING, WIDTH - PADDING);
        ypos = glm::clamp(ypos, PADDING, HEIGHT - PADDING);
        glm::vec2 cursor(xpos, HEIGHT - ypos);
        *selectPtr = cursor;
    }
}

static void KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS &&
        userPts.size() < MAX_POINTS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = glm::clamp(xpos, PADDING, WIDTH - PADDING);
        ypos = glm::clamp(ypos, PADDING, HEIGHT - PADDING);

        userPts.push_back(glm::vec2(xpos, HEIGHT - ypos));
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        glm::vec2* p = SelectPoints();
        glm::vec2* v = userPts.data();
        if (p >= v && p < v + userPts.size()) {
            int offset = p - v;
            userPts.erase(userPts.begin() + offset);
        }
    }
    else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        if (wireMesh) {
            wireMesh = false;
            maxSteps = 1000;
        }
        else {
            wireMesh = true;
            maxSteps = 1;
        }
    }
    else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        boldMesh ^= 1;
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        drawShadow ^= 1;
    }
    else if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        if (!intersectMode) {
            intersectMode = true;
            a = glm::vec2(640, 340);
            b = glm::vec2(640, 680);
            maxSteps = 1;
            wireMesh = true;
        }
        else {
            intersectMode = false;
            depth = 0;
            wireMesh = 1;
            boldMesh = 0;
            drawShadow = 1;
        }
    }
    else if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, 1);
    }
}

static void ScrollCallback(GLFWwindow*, double, double yoff)
{
    if (intersectMode) {
        if (yoff > 0) {
            ++maxSteps;
        }
        else {
            --maxSteps;
        }
        maxSteps = glm::clamp(maxSteps, 1, 1000);
    }
    else {
        if (yoff > 0) {
            ++depth;
        }
        else {
            --depth;
        }
        depth = glm::clamp(depth, 0, MAX_DEPTH);
    }
}

void Clear(glm::vec3 color = black)
{
    int bufsiz = WIDTH * HEIGHT;
    for (int i = 0; i < bufsiz; ++i) {
        buf[i] = color;
    }
}

void DrawPoint(glm::ivec2 p, glm::vec3 color = darkRed)
{
    const float rad = 8;
    const float edge = 10;
    assert(p.x >= edge && p.x < WIDTH - edge);
    assert(p.y >= edge && p.y < HEIGHT - edge);

    for (int yoff = -edge; yoff <= edge; ++yoff) {
        int index = (p.y + yoff) * WIDTH + (p.x - edge);
        for (int xoff = -edge; xoff <= edge; ++xoff) {
            float dst = hypot(xoff, yoff);
            float t = glm::smoothstep(rad, edge, dst);
            buf[index] = buf[index] * t + color * (1.f - t);
            glm::mix(color, buf[index], t);
            index++;
        }
    }
}

void DrawHorzLine(glm::ivec2 p, int xto, const int rad = 0,
                  glm::vec3 color = white)
{
    if (p.x > xto)
        std::swap(p.x, xto);

    const int edge = rad + 2;

    for (int yoff = -edge + 1; yoff < edge; ++yoff) {
        float t = glm::smoothstep(rad, edge, glm::abs(yoff));
        int index = (p.y + yoff) * WIDTH + p.x;
        for (int x = p.x; x <= xto; ++x) {
            if (glm::length(buf[index] - darkYellow) > 0.08f)
                buf[index] = glm::mix(color, buf[index], t);
            // TODO
            ++index;
        }
    }
}

void DrawVertLine(glm::ivec2 p, int yto, const int rad = 0,
                  glm::vec3 color = white)
{
    if (yto < p.y)
        std::swap(yto, p.y);

    const int edge = rad + 2;
    assert(p.x >= rad && p.x < WIDTH - rad);

    for (int y = p.y; y <= yto; ++y) {
        int index = y * WIDTH + (p.x - edge + 1);
        for (int xoff = -edge + 1; xoff < edge; ++xoff) {
            float t = glm::smoothstep(rad, edge, glm::abs(xoff));
            if (glm::length(buf[index] - darkYellow) > 0.08f)
                buf[index] = glm::mix(color, buf[index], t);
            // TODO
            ++index;
        }
    }
}

void DrawRectangle(glm::ivec2 p1, glm::ivec2 p2, glm::vec3 color = white)
{
    int rad = boldMesh ? 1 : 0;
    if (boldMesh)
        color = white;

    glm::ivec2 pmin = glm::min(p1, p2);
    glm::ivec2 pmax = glm::max(p1, p2);
    DrawHorzLine(pmin, pmax.x, rad, color);
    DrawVertLine(pmin, pmax.y, rad, color);
    DrawHorzLine(pmax, pmin.x, rad, color);
    DrawVertLine(pmax, pmin.y, rad, color);
}

void DrawLine(glm::vec2 start, glm::vec2 end, glm::vec3 color = white)
{
    const float rad = 0;
    const float edge = 2;

    glm::ivec2 vmin = glm::floor(glm::min(start, end));
    glm::ivec2 vmax = glm::ceil(glm::max(start, end));

    glm::vec2 v = glm::normalize(end - start);
    glm::vec2 n(-v.y, v.x);
    float c = -glm::dot(start, n);

    if (glm::abs(n.x) < 1e-5)
        return DrawHorzLine(start, end.x, 0, color);
    float xrad = glm::abs(edge / n.x);

    for (int y = vmin.y; y <= vmax.y; ++y) {
        float xmid = (-n.y * y - c) / n.x;
        int xfrom = glm::clamp((int)glm::floor(xmid - xrad), vmin.x, vmax.x);
        int xto = glm::clamp((int)glm::ceil(xmid + xrad), vmin.x, vmax.x);
        int index = y * WIDTH + xfrom;
        for (int x = xfrom; x <= xto; ++x) {
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
    for (int i = 0; i < nts; ++i) {
        float t = ts[i];
        for (int j = 0; j < nts - i; ++j) {
            buf[j] = glm::mix(cpy[j], cpy[j + 1], t);
        }
        std::swap(cpy, buf);
    }
    return cpy[0];
}

glm::vec2 BlossomBezier(const std::vector<glm::vec2>& pts, float t)
{
    float ts[MAX_POINTS];
    int nts = pts.size() - 1;
    for (int i = 0; i < nts; ++i) {
        ts[i] = t;
    }
    return BlossomBezier(pts, ts);
}

glm::vec2 BlossomBezier(const std::vector<glm::vec2>& pts, float uFrom,
                        float uTo, int n)
{
    float ts[MAX_POINTS];
    int nts = pts.size() - 1;
    for (int i = 0; i < nts; ++i) {
        if (i < n)
            ts[i] = uTo;
        else
            ts[i] = uFrom;
    }
    return BlossomBezier(pts, ts);
}

void DrawCurveBase(const std::vector<glm::vec2>& pts, const float rad,
                   glm::vec3 color = blue)
{
    const float edge = rad + 2.0;

    glm::vec2 vmin = glm::min(pts.front(), pts.back());
    glm::vec2 vmax = glm::max(pts.front(), pts.back());
    vmin = glm::floor(vmin - glm::vec2(edge));
    vmax = glm::ceil(vmax + glm::vec2(edge));

    glm::vec2 dir = pts.back() - pts.front();
    float len = glm::length(dir);
    glm::vec2 n = glm::vec2(-dir.y, dir.x) / len;
    float c = -glm::dot(n, pts.front());

    glm::vec2 derStart = pts[1] - pts[0];
    glm::vec2 derEnd = pts[pts.size() - 1] - pts[pts.size() - 2];

    for (int y = vmin.y; y <= vmax.y; ++y) {
        for (int x = vmin.x; x <= vmax.x; ++x) {
            glm::vec2 p(x, y);
            if (glm::dot(p - pts.front(), derStart) < -1e-5)
                continue;
            if (glm::dot(p - pts.back(), derEnd) > 1e-5)
                continue;

            float signDst = glm::dot(n, p) + c;
            glm::vec2 proj = p - signDst * n;
            float u = glm::dot(proj - pts.front(), dir) / (len * len);
            u = glm::clamp(0.f, 1.f, u);
            glm::vec2 closest = BlossomBezier(pts, u);

            float t = glm::smoothstep(rad, edge, glm::distance(closest, p));
            int index = y * WIDTH + x;
            buf[index] = glm::mix(color, buf[index], t);
        }
    }
}

void DrawCurve(const std::vector<glm::vec2>& pts, int d, int maxd,
               glm::vec3 color)
{
    if (d >= maxd) {
        if (wireMesh && d < MAX_DEPTH) {
            glm::vec2 pmin(INFINITY), pmax(-INFINITY);
            for (const glm::vec2& p : pts) {
                pmin = glm::min(pmin, p);
                pmax = glm::max(pmax, p);
            }
            DrawRectangle(pmin, pmax, gray);
        }

        if (d == MAX_DEPTH)
            DrawCurveBase(pts, 3.f, color);
        else
            DrawLine(pts.front(), pts.back(), color);
        return;
    }

    std::vector<glm::vec2> subdiv;
    for (int i = 0; i < (int)pts.size(); ++i) {
        subdiv.push_back(BlossomBezier(pts, 0.f, 0.5f, i));
    }
    DrawCurve(subdiv, d + 1, maxd, color);

    subdiv.resize(0);
    for (int i = 0; i < (int)pts.size(); ++i) {
        subdiv.push_back(BlossomBezier(pts, 0.5f, 1.f, i));
    }

    DrawCurve(subdiv, d + 1, maxd, color);
}

bool SegmentBoxIntersection(glm::vec2 vmin, glm::vec2 vmax)
{
    glm::vec2 v = glm::normalize(b - a);
    glm::vec2 n(-v.y, v.x);
    float c = -glm::dot(n, a);

    if (glm::abs(n.y) < 1e-5) {
        assert(glm::abs(n.x) > 1e-5);
        float x = -c / n.x;
        return vmin.x < x && x < vmax.x &&
               !(vmin.y > glm::max(a.y, b.y) || vmax.y < glm::min(a.y, b.y));
    }
    if (glm::abs(n.x) < 1e-5) {
        assert(glm::abs(n.y) > 1e-5);
        float y = -c / n.y;
        return vmin.y < y && y < vmax.y &&
               !(vmin.x > glm::max(a.x, b.x) || vmax.x < glm::min(a.x, b.x));
    }

    float y0 = -(c + n.x * vmin.x) / n.y;
    float y1 = -(c + n.x * vmax.x) / n.y;
    if (y0 > y1)
        std::swap(y0, y1);

    float lsect0 = glm::max(vmin.y, y0);
    float lsect1 = glm::min(vmax.y, y1);
    return lsect0 < lsect1 &&
           !(lsect0 > glm::max(a.y, b.y) || lsect1 < glm::min(a.y, b.y));
}

void Intersect(const std::vector<glm::vec2>& pts, int& steps)
{
    glm::vec2 pmin(INFINITY), pmax(-INFINITY);
    for (const glm::vec2& p : pts) {
        pmin = glm::min(pmin, p);
        pmax = glm::max(pmax, p);
    }

    if (!SegmentBoxIntersection(pmin, pmax)) {
        if (wireMesh)
            DrawRectangle(pmin, pmax, darkRed);
        return;
    }
    if (glm::distance2(pmin, pmax) < 1.f) {
        glm::vec2 mid = (pts.front() + pts.back()) / 2.f;
        DrawPoint(mid, darkYellow);
        return;
    }
    if (steps <= 0) {
        if (wireMesh)
            DrawRectangle(pmin, pmax, green);
        return;
    }
    --steps;

    std::vector<glm::vec2> subdiv;
    for (int i = 0; i < (int)pts.size(); ++i) {
        subdiv.push_back(BlossomBezier(pts, 0.f, 0.5f, i));
    }
    Intersect(subdiv, steps);

    subdiv.resize(0);
    for (int i = 0; i < (int)pts.size(); ++i) {
        subdiv.push_back(BlossomBezier(pts, 0.5f, 1.f, i));
    }
    Intersect(subdiv, steps);
}

void IntersectMode()
{
    Clear();
    if (userPts.size() >= 2) {
        DrawCurve(userPts, 0, MAX_DEPTH, gray);
    }
    if (userPts.size() >= 2) {
        int steps = maxSteps - 1;
        Intersect(userPts, steps);
        assert(steps >= 0);
        if (wireMesh)
            maxSteps -= steps;
    }
    DrawLine(a, b, darkYellow);
    DrawPoint(a, darkYellow);
    DrawPoint(b, darkYellow);
    if (userPts.size() >= 1) {
        for (int i = 0; i < (int)userPts.size(); ++i) {
            DrawPoint(userPts[i]);
            RenderText(buf, i, userPts[i]);
        }
    }

    SwapBuffers(window, buf);
}

void DrawMode()
{
    Clear();
    if (userPts.size() >= 2) {
        if (depth < MAX_DEPTH && drawShadow)
            DrawCurve(userPts, 0, MAX_DEPTH, gray);
        DrawCurve(userPts, 0, depth, blue);
    }
    if (userPts.size() >= 1) {
        for (int i = 0; i < (int)userPts.size(); ++i) {
            DrawPoint(userPts[i]);
            RenderText(buf, i, userPts[i]);
        }
    }
    SwapBuffers(window, buf);
}

int main()
{
    window = CreateRenderer(WIDTH, HEIGHT);
    if (window == NULL) {
        exit(EXIT_FAILURE);
    }
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    int bufsiz = WIDTH * HEIGHT;
    buf = (glm::vec3*)malloc(bufsiz * sizeof(glm::vec3));

    InitRenderText();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (intersectMode)
            IntersectMode();
        else
            DrawMode();
    }

    free(buf);
    DestroyRenderer(window);
    return 0;
}
