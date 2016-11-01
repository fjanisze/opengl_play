#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include "logger/logger.hpp"
#include "shaders.hpp"
#include "text_rendering.hpp"


namespace opengl_play
{

const std::string simple_vertex_shader = {
    "#version 330 core\n"
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec3 selected_color;\n"
        "out vec3 color;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(position, 1.0f);\n"
        "color = selected_color;\n"
        "}\0"
};

const std::string simple_fragment_shader = {
    "#version 330 core\n"
        "out vec4 final_color;\n"
        "in vec3 color;\n"
        "void main()\n"
        "{\n"
        "final_color = vec4(color,1.0f);\n"
        "}\n\0"
};

const std::string simple_fragment_shader2 = {
    "#version 330 core\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "color = vec4(0.7f, 0.7f, 0.7f, 1.0f);\n"
        "}\n\0"
};

enum class rotation_direction
{
    left,
    right
};

#define AMOUNT_OF_POINTS 120*4

class opengl_ui
{
    bool       render_update_needed;
    GLFWwindow* window_ctx;
    int win_h,
        win_w;
    void setup_callbacks();
    void get_current_ctx_viewport();
    int check_for_errors();
    static void mouse_click_callback(GLFWwindow* ctx,
                                     int button,
                                     int action,
                                     int);
    static void cursor_pos_callback(GLFWwindow* ctx,
                                    double x,
                                    double y);
    static void window_resize_callback(GLFWwindow* ctx,
                                       int width,
                                       int height);
public:
    opengl_ui(int win_width, int win_heigth);
    void        prepare_for_main_loop();
    void        enter_main_loop();
    GLFWwindow *get_win_ctx();
    void        image_update_needed();
    void        update_viewport(int new_win_h,
                                int new_win_w);
    virtual ~opengl_ui();
};

}
