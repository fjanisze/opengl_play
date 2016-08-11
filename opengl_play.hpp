#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include "logger/logger.hpp"


namespace opengl_play
{

const std::string simple_vertex_shader = {
    "#version 330 core\n"
        "layout (location = 0) in vec3 position;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
        "}\0"
};

const std::string simple_fragment_shader = {
    "#version 330 core\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0"
};

class my_small_shaders
{
    GLuint vertex_shader,
           fragment_shader,
           shader_program;
    GLchar log_buffer[512];
    void load_shader_generic(GLuint& shader_target,
                             const std::string& body,
                             GLenum shader_type);
public:
    my_small_shaders();
    void load_vertex_shader(const std::string& body);
    void load_fragment_shader(const std::string& body);
    bool create_shader_program();
    void use_shaders();
};

class opengl_ui
{
    GLFWwindow* window_ctx;
    my_small_shaders shaders;
    int win_h,
        win_w;
    void setup_callbacks();
    void update_viewport();
public:
    opengl_ui(int win_width, int win_heigth);
    void enter_main_loop();
    virtual ~opengl_ui();
};

}
