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
    glm::ivec2 my_triangle[3];
    int        points_count;
    GLfloat    vertices[ 3 * 3 + AMOUNT_OF_POINTS * 3];
    GLFWwindow* window_ctx;
    shaders::my_small_shaders shaders,
                              shader2;
    text_renderer::rendr_text random_text,
                              position_info_text;
    int win_h,
        win_w;
    void setup_callbacks();
    void update_viewport();
    void init_my_triangle();
    void rotate_triangle(rotation_direction dir, int amount);
    void update_vertices();
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
