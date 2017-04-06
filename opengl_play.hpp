#include <headers.hpp>
#include "logger/logger.hpp"
#include "shaders.hpp"
#include "text_rendering.hpp"
#include <opengl_object.hpp>
#include "my_camera.hpp"
#include "my_lines.hpp"
#include "lights.hpp"
#include <movable_object.hpp>
#include <random>
#include <models.hpp>
#include <terrains.hpp>
#include <types.hpp>
#include <map_entities.hpp>
#include <framebuffers.hpp>

namespace opengl_play
{

void mouse_click_callback(GLFWwindow* ctx,
                          int button,
                          int action,
                          int);
void cursor_pos_callback(GLFWwindow* ctx,
                         double x,
                         double y);
void window_resize_callback(GLFWwindow* ctx,
                            int width,
                            int height);

static const int stat_key_array_size{ 1024 };
enum class key_status_t
{
    pressed,
    not_pressed
};

class opengl_ui
{
    void setup_callbacks();
    void get_current_ctx_viewport();
    void init_text();
    void evaluate_key_status();
    void setup_scene();
    /*
     * Verify whether the mouse is over one of the
     * models rendered in models_back_buffer, if so
     * trigger a mouse_hover call
     */
    void verify_models_intersections( GLfloat x, GLfloat y );
public:
    opengl_ui(int win_width, int win_heigth);
    void        prepare_for_main_loop();
    void        enter_main_loop();
    GLFWwindow *get_win_ctx();
    void        update_viewport(int new_win_h,
                                int new_win_w);
    void		ui_mouse_click(GLint button,GLint action);
    void        ui_mouse_move(GLdouble x,GLdouble y);
    void		ui_mouse_enter_window(int state);
    void		ui_wheel_move(GLdouble x,GLdouble y);
    void		ui_keyboard_press(GLint button,GLint scode,GLint action);
    virtual ~opengl_ui();
private:
    key_status_t  key_status[stat_key_array_size];
    GLFWwindow*   window_ctx;
    glm::mat4	  projection;
    glm::vec4     viewport;
    little_object_ptr object;
    GLFWcursor*   cursor;
    int			  mouse_line_idx;
    text_renderer::rendr_text fps_info,
    camera_info;
    renderable::renderer_pointer renderer;

    int win_h,
    win_w;
    camera_ptr		camera;
    Framebuffers::framebuffers_ptr frame_buffers;
    GLfloat models_back_buffer;
    /*
     * Ray casting utility
     */
    types::ray_t ray_cast( GLdouble x, GLdouble y );
    glm::vec2    ray_z_hit_point( const types::ray_t& ray,
                                  const GLfloat z_value );

    my_lines_ptr position_lines;
    terrains::terrains_ptr game_terrain;
    map_entities::entity_collection_ptr game_map_entities;
    lights::generic_light_ptr light_1,
                            light_2,
                            flash_light;
    map_entities::model_id my_car;
    GLfloat mouse_x_pos;
    GLfloat mouse_y_pos;

    movable::object_movement_processor movement_processor;
};

}
