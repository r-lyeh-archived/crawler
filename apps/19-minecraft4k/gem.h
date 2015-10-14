#ifndef GEM_H_INCLUDED
#define GEM_H_INCLUDED

// a simple wrapper around GLFW for the Minecraft4k code
// this code is public domain - use at your own risk

#include <GLFW/glfw3.h>
#include <cmath>
#include <ctime>
#include <vector>
#include <stdint.h>
#include <iostream>


class gem { // graphics environment manager
public:
    gem(int frame_width, int frame_height, int scale)
    : good_(false), width_(frame_width), height_(frame_height),
      scale_(scale), fps_counter_(0), clock_(0), window_(0)
    {
        frame_buf_.resize(width_ * height_);
        if (glfwInit()) {
            good_ = true;
            window_ = glfwCreateWindow(width_*scale_, height_*scale_,
                "mc4k - press Esc to exit", 0, 0);
            if (window_) {
                glfwSetKeyCallback(window_, key_callback);
                glfwMakeContextCurrent(window_);
                glfwSwapInterval(1);
            }
        }
    }
    
    ~gem()
    {
        if (good_)
            glfwTerminate();
    }
    
    typedef void renderer(void * private_data, uint32_t * frame_buf);

    // repeatedly display frames until user quits
    void run(renderer * render, void * private_renderer_data)
    {
        while (good_ && window_) {
            render(private_renderer_data, &frame_buf_[0]);
            glClear(GL_COLOR_BUFFER_BIT);
            glPixelZoom((float)scale_, (float)scale_);
            glDrawPixels(width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, &frame_buf_[0]);
            glfwSwapBuffers(window_);
            glfwPollEvents();
            if (glfwWindowShouldClose(window_))
                break;

            // display the frames per second count every second
            ++fps_counter_;
            if (clock() - clock_ > CLOCKS_PER_SEC) {
                std::cerr << fps_counter_ << " FPS\n";
                fps_counter_ = 0;
                clock_ = clock();
            }
        }
    }
    
private:
    bool good_; // good_ <=> GLFW library initialised successfully
    int width_; // frame buffer width in pixels
    int height_; // frame buffer height in pixels
    int scale_; // each pxl in frame buf displayed as scale_ x scale_ pxls on screen
    int fps_counter_; // running count of frames this second
    clock_t clock_; // time of last FPS printout
    GLFWwindow * window_; // the GLFW window handle
    std::vector<uint32_t> frame_buf_; // width_ x height_ pixels

    static void key_callback(GLFWwindow * window, int key, int, int action, int)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }
};


#endif
