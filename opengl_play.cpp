#include "opengl_play.hpp"

namespace opengl_play
{

namespace
{
//Saved context for the GLFW callbacks
opengl_ui* ui_instance;
//Global for the mouse callback
int left_button_state = GLFW_RELEASE;
//This coordinate is needed to calculate the delta movement
glm::dvec2 last_know_mouse_position = {0,0};
}

GLfloat mycos(double val){
	return (GLfloat)cos(val) * 0.5;
}


GLfloat mysin(double val){
	return (GLfloat)sin(val) * 0.5;
}

GLfloat torad(double val){
	return val * M_PI/180;
}

void mouse_click_callback(GLFWwindow *ctx,
						  int button,
						  int action,
						  int)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if(action == GLFW_PRESS)
		{
			left_button_state = action;
			glfwGetCursorPos(ui_instance->get_win_ctx(),
							 &last_know_mouse_position.x,
							 &last_know_mouse_position.y);
		}
		else
			left_button_state = GLFW_RELEASE;
	}
	ui_instance->ui_mouse_click(button,action);
	ui_instance->image_update_needed();
}

void cursor_pos_callback(GLFWwindow *ctx,
						 double x,
						 double y)
{
	if(left_button_state == GLFW_PRESS)
	{
		last_know_mouse_position.x = x;
		last_know_mouse_position.y = y;
	}
}

void window_resize_callback(GLFWwindow *ctx,
							int width,
							int height)
{
	ui_instance->update_viewport(height,width);
	ui_instance->image_update_needed();
}

void keyboard_press_callback(GLFWwindow* ctx,
							 int key,
							 int scode,
							 int action,
							 int)
{
	ui_instance->ui_keyboard_press(key,scode,action);
}

void opengl_ui::ui_mouse_click(GLint button, GLint action)
{
	object->mouse_click(button,action);
}

void opengl_ui::ui_keyboard_press(GLint button,
								  GLint scode,
								  GLint action)
{
	static std::unordered_map<GLint,mov_direction> mapping = {
		{GLFW_KEY_UP,mov_direction::top},
		{GLFW_KEY_DOWN,mov_direction::down},
		{GLFW_KEY_LEFT,mov_direction::left},
		{GLFW_KEY_RIGHT,mov_direction::right}
	};
	if(action == GLFW_PRESS || action == GLFW_REPEAT) {
		auto it = mapping.find(button);
		if( it != mapping.end() ) {
			object->move(it->second,0.1);
			render_update_needed = true;
		} else {
			if( button == GLFW_KEY_A ) {
				object->image_rotation(0.1);
				render_update_needed = true;
			} else if( button == GLFW_KEY_D ) {
				object->image_rotation(-0.1);
				render_update_needed = true;
			}
			if( button == GLFW_KEY_W ) {
				object->scale(0.9);
				render_update_needed = true;
			} else if( button == GLFW_KEY_E ) {
				object->scale(1.1);
				render_update_needed = true;
			}
		}
	}
}


void opengl_ui::setup_callbacks()
{

	glfwSetMouseButtonCallback(window_ctx,
							   mouse_click_callback);
	glfwSetCursorPosCallback(window_ctx,
							 cursor_pos_callback);
	glfwSetWindowSizeCallback(window_ctx,
							  window_resize_callback);
	glfwSetKeyCallback(window_ctx,
					   keyboard_press_callback);
}

void opengl_ui::get_current_ctx_viewport()
{
	glfwGetFramebufferSize(window_ctx,
						   &win_w,
						   &win_h);
	glViewport(0,0,
			   win_w,
			   win_h);
}

void opengl_ui::init_fps_info()
{
	fps_info = std::make_shared<text_renderer::renderable_text>();
	fps_info->set_position(glm::fvec2(10,10));
	fps_info->set_color(glm::vec3(1.0f,1.0f,1.0f));
	fps_info->set_scale(1.0f);
	fps_info->set_text("0 fps");
	fps_info->set_window_size(win_h,win_w);
}

opengl_ui::opengl_ui(int win_width,
					 int win_heigth) :
	win_h{ win_heigth },
	win_w{ win_width }
{
	LOG1("Creating window, ",win_width,"/",win_heigth);
	//Setup GLFW
	if(glfwInit() != GLFW_TRUE)
	{
		ERR("Unable to initialize GLFW");
		throw std::runtime_error("GLFW init failed");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create the window
	window_ctx = glfwCreateWindow(win_width,
								  win_heigth,
								  "texture play",
								  nullptr,
								  nullptr);
	if(window_ctx == nullptr){
		ERR("Unable to create the window!");
		throw std::runtime_error("Window creation failed");
	}

	glfwMakeContextCurrent(window_ctx);

	//Init GLEW
	glewExperimental = GL_TRUE;
	GLenum glew_init_status = glewInit();
	if(glew_init_status != GLEW_OK){
		ERR("Unable to initialize GLEW: ",
			glewGetErrorString(glew_init_status));
		throw std::runtime_error("GLEW Init failed!");
	}

	object = std::make_shared<little_object>();

	init_fps_info();
}

void opengl_ui::prepare_for_main_loop()
{
	LOG3("opengl_ui, preparing the remaining environment.");
	check_for_errors();
	//Save the instance pointer
	ui_instance = this;

	//Set to true if there's something to update
	//is true now since we need an initial update
	render_update_needed = true;
	//Init the callbacks
	setup_callbacks();

	get_current_ctx_viewport();
}

void opengl_ui::enter_main_loop()
{
	check_for_errors();
	//now we have our object configured and ready to be rendered

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto ref_time = std::chrono::system_clock::now();
	int  current_fps = 0;

	LOG2("Entering main loop!");
	while(!glfwWindowShouldClose(window_ctx))
	{
		++current_fps;
		glfwPollEvents();

		//Let's rotate the image
		//object->image_rotation(0.1f);

		auto current_time = std::chrono::system_clock::now();
		if(std::chrono::duration_cast<
				std::chrono::milliseconds>(
					current_time - ref_time).count() > 1000){
			ref_time = current_time;
			fps_info->set_text(std::to_string(current_fps) + "fps");
			current_fps = 0;
		}


		//Do not draw anything if is not needed
		if(false == render_update_needed)
		{
			//Disabled in order to have the fps printed :(
			//continue;
		}

		glClearColor(0.5,0.5,0.5,1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		object->prepare_for_render();
		object->render();
		object->clean_after_render();

		fps_info->render_text();

		glfwSwapBuffers(window_ctx);

		//Till the next update
		render_update_needed = false;
	}
}

GLFWwindow *opengl_ui::get_win_ctx()
{
	return window_ctx;
}

void opengl_ui::image_update_needed()
{
	render_update_needed = true;
}

void opengl_ui::update_viewport(int new_win_h,
								int new_win_w)
{
	win_h = new_win_h;
	win_w = new_win_w;

	glViewport(0,0,
			   win_w,
			   win_h);
}

opengl_ui::~opengl_ui()
{
	if(window_ctx!=nullptr){
		glfwTerminate();
	}
}

}

int main()
{
	opengl_play::opengl_ui entry(1024,768);
	log_inst.set_thread_name("MAIN");

	entry.prepare_for_main_loop();
	entry.enter_main_loop();
}
