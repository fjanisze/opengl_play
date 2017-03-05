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
public:
	opengl_ui(int win_width, int win_heigth);
	void        prepare_for_main_loop();
	void        enter_main_loop();
	GLFWwindow *get_win_ctx();
	void        update_viewport(int new_win_h,
								int new_win_w);
	void		ui_mouse_click(GLint button,GLint action);
	void        ui_mouse_move(GLdouble x,GLdouble y);
	void		ui_wheel_move(GLdouble x,GLdouble y);
	void		ui_keyboard_press(GLint button,GLint scode,GLint action);
	virtual ~opengl_ui();
public:
	key_status_t  key_status[stat_key_array_size];
	GLFWwindow*   window_ctx;
	little_object_ptr object;
	GLFWcursor*   cursor;
	text_renderer::rendr_text fps_info,
						camera_info;
	int win_h,
		win_w;
	camera_obj		camera;

	my_lines_ptr position_lines;
	shaders::my_small_shaders model_shader;
	models::model_ptr terrain;
	lights::generic_light_ptr light_1,
							flash_light;

	movable::object_movement_processor movement_processor;
};

}
