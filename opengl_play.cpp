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
	static std::unordered_map<GLint,mov_direction> moving_mapping = {
		{GLFW_KEY_UP,mov_direction::top},
		{GLFW_KEY_DOWN,mov_direction::down},
		{GLFW_KEY_LEFT,mov_direction::left},
		{GLFW_KEY_RIGHT,mov_direction::right}
	};
	static std::unordered_map<GLint, std::pair<rotation_axis,GLfloat>> rot_mapping{
		{GLFW_KEY_D,{rotation_axis::z,-0.1}},
		{GLFW_KEY_A,{rotation_axis::z,0.1}},
		{GLFW_KEY_W,{rotation_axis::x,-0.1}},
		{GLFW_KEY_S,{rotation_axis::x,0.1}},
		{GLFW_KEY_Q,{rotation_axis::y,-0.1}},
		{GLFW_KEY_E,{rotation_axis::y,0.1}},
	};
	if(action == GLFW_PRESS || action == GLFW_REPEAT) {
		auto it = moving_mapping.find(button);
		if( it != moving_mapping.end() ) {
			object->move(it->second,0.1);
			return;
		}
		auto it2 = rot_mapping.find(button);
		if( it2 != rot_mapping.end() ) {
			object->image_rotation(it2->second.first,it2->second.second);
			return;
		}
		switch( button ) {
		case GLFW_KEY_F:
			object->scale(0.9);
			break;
		case GLFW_KEY_V:
			object->scale(1.1);
			break;
		default:
			break;
		}
		render_update_needed = true;
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

	glEnable(GL_DEPTH_TEST);

	auto ref_time = std::chrono::system_clock::now();
	int  current_fps = 0;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)win_w / (GLfloat)win_h, 0.1f, 100.0f);

	glm::vec3 cube_position[] = {
		glm::vec3( -3.0f,  4.0f,  -10.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	LOG2("Entering main loop!");
	while(!glfwWindowShouldClose(window_ctx))
	{
		++current_fps;
		glfwPollEvents();

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		object->prepare_for_render();
		object->set_transformations(model,view,projection);

		//Let's add some more random cubes
		for( int i{0} ; i<10 ; ++i ) {
			//Render the current cube
			object->render();
			//New position:
			glm::mat4 model_location;
			model_location = glm::translate(model,
											cube_position[i]);
			object->set_transformations(model_location,
										   view,
										   projection);
		}

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
