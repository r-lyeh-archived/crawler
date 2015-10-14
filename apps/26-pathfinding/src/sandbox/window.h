#pragma once

#include "linalg.h"
#include <functional>
#include <GLFW\glfw3.h>

class Window
{
    GLFWwindow * window = 0;
    GLuint fontTexture = 0;
    std::function<void(int,int,int,int)> keyHandler;

    static void OnKey(GLFWwindow*,int,int,int,int);

    Window() {}
public:
    Window(const int2 & dimensions, const char * title);
    ~Window();

    bool WindowShouldClose() const { return !!glfwWindowShouldClose(window); }
    int2 GetFramebufferSize() const { int2 r; glfwGetFramebufferSize(window, &r.x, &r.y); return r; }
    double2 GetCursorPos() const { double2 r; glfwGetCursorPos(window, &r.x, &r.y); return r; }
    bool GetMouseButton(int button) const { return glfwGetMouseButton(window, button) == GLFW_PRESS; }
    bool GetKey(int key) const { return glfwGetKey(window, key) == GLFW_PRESS; }

    void SetKeyHandler(std::function<void(int key, int scancode, int action, int mods)> handler) { keyHandler = handler; }

    void MakeContextCurrent() { glfwMakeContextCurrent(window); }
    void Print(const int2 & coord, const char * format, ...);
    void SwapBuffers() { glfwSwapBuffers(window); }
};