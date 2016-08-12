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

enum class rotation_direction
{
    left,
    right
};

class opengl_ui
{

    glm::ivec2 my_triangle[3];
    GLfloat    vertices[9];
    GLFWwindow* window_ctx;
    my_small_shaders shaders;
    int win_h,
        win_w;
    void setup_callbacks();
    void update_viewport();
    void init_my_triangle();
    void rotate_triangle(rotation_direction dir, int amount);
    void update_vertices();
    static void mouse_click_callback(GLFWwindow* ctx,
                                     int button,
                                     int action,
                                     int);
    static void cursor_pos_callback(GLFWwindow* ctx,
                                    double x,
                                    double y);
public:
    opengl_ui(int win_width, int win_heigth);
    void enter_main_loop();
    GLFWwindow *get_win_ctx();
    virtual ~opengl_ui();
};

}
