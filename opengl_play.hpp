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

int check_for_errors();

const std::string lit_ob_vertex_sh = "#version 330 core\n"
	"layout (location = 0) in vec3 position;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
	"}\0";

const std::string lit_ob_frag_sh = "#version 330 core\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\n\0";

class little_object
{
	GLuint VAO,VBO,EBO;
	glm::vec3 vertices[4];
	GLuint    vertex_idxs[6];
	GLfloat   vertex_data[12];
	shaders::my_small_shaders obj_shader;

	void init_vertices();
public:
	little_object();
	~little_object();
	void prepare_for_render();
	void render();
	void clean_after_render();
};

using little_object_ptr = std::shared_ptr<little_object>;

class opengl_ui
{
	bool          render_update_needed;
	GLFWwindow*   window_ctx;
	little_object_ptr object;
    int win_h,
        win_w;
    void setup_callbacks();
    void get_current_ctx_viewport();
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
