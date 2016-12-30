#include "opengl_play.hpp"

namespace opengl_play
{

namespace
{
//Saved context for the GLFW callbacks
opengl_ui* ui_instance;
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
	ui_instance->ui_mouse_click(button,action);
}

void cursor_pos_callback(GLFWwindow *ctx,
						 double x,
						 double y)
{
	ui_instance->ui_mouse_move(x,y);
}

void window_resize_callback(GLFWwindow *ctx,
							int width,
							int height)
{
	ui_instance->update_viewport(height,width);
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
	if(action != GLFW_PRESS)
		return;
	if(button == GLFW_MOUSE_BUTTON_LEFT) {
		object->texture_mix(true);
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT) {
		object->texture_mix(false);
	}
}

void opengl_ui::ui_mouse_move(GLdouble x, GLdouble y)
{
	static bool first_move = true;
	if(first_move) {
		last_mouse_x = x;
		last_mouse_y = y;
		first_move = false;
	}
	GLdouble x_delta = (x - last_mouse_x) * 0.1,
			y_delta = (last_mouse_y - y) * 0.1;

	camera->rotate_camera(y_delta,x_delta);
	object->modify_view(camera->get_view());
	position_lines->modify_view(camera->get_view());

	last_mouse_x = x;
	last_mouse_y = y;
}

void opengl_ui::ui_keyboard_press(GLint button,
								  GLint scode,
								  GLint action)
{
	if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
		key_status[ button ] = key_status_t::pressed;
	} else if( action == GLFW_RELEASE ) {
		key_status[ button ] = key_status_t::not_pressed;
	}
}

void opengl_ui::evaluate_key_status()
{
	static std::unordered_map<GLint,mov_direction> cam_moving_mapping = {
		{GLFW_KEY_W,mov_direction::top},
		{GLFW_KEY_S,mov_direction::down},
		{GLFW_KEY_A,mov_direction::left},
		{GLFW_KEY_D,mov_direction::right}
	};
	for(int button{ 0 } ; button < stat_key_array_size ; ++button){
		if( key_status[ button ] == key_status_t::pressed ) {
			//Camera moving
			auto it = cam_moving_mapping.find(button);
			if( it != cam_moving_mapping.end() ) {
				camera->move_camera( it->second, 0.1 );
			}
			position_lines->modify_view(camera->get_view());
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

void opengl_ui::init_text()
{
	fps_info = std::make_shared<text_renderer::renderable_text>();
	fps_info->set_position(glm::fvec2(10,7));
	fps_info->set_color(glm::vec3(1.0f,1.0f,1.0f));
	fps_info->set_scale(0.3f);
	fps_info->set_text("0 fps");
	fps_info->set_window_size(win_h,win_w);

	camera_info = std::make_shared<text_renderer::renderable_text>();
	camera_info->set_position(glm::fvec2(win_h - 200,7));
	camera_info->set_color(glm::vec3(1.0f,1.0f,1.0f));
	camera_info->set_scale(0.3f);
	camera_info->set_text(" -- ");
	camera_info->set_window_size(win_h,win_w);
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
								  "light play",
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
	position_lines = std::make_shared<my_static_lines>();

	init_text();

	camera = my_camera::create_camera({2.0,2.0,10.0},{1.0,1.0,1.0});

	for(auto& elem:key_status)
		elem = key_status_t::not_pressed;
}

void opengl_ui::prepare_for_main_loop()
{
	LOG3("opengl_ui, preparing the remaining environment.");
	check_for_errors();
	//Save the instance pointer
	ui_instance = this;
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
	glfwSetInputMode(window_ctx, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	auto ref_time = std::chrono::system_clock::now();
	int  current_fps = 0;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f),
						(GLfloat)win_w / (GLfloat)win_h,
						0.1f, 100.0f);

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
		glm::vec3(-1.3f,  1.0f, -1.5f),
		glm::vec3( 0.0f,  0.0f, 0.0f)
	};

	int obj_id = object->add_object(glm::vec3(0.0,0.0,0.0),
									glm::vec3(1.0,0.5,0.31));
	object->select_object(obj_id);

	position_lines->modify_model(model);
	position_lines->modify_projection(projection);

	position_lines->modify_view(camera->get_view());

	camera->update_cam_view();

	//Add the lines
	position_lines->add_line({0.0,0.0,0.0},
							{10.0,0.0,0.0},{1.0,0.0,0.0});
	position_lines->add_line({0.0,0.0,0.0},
							{0.0,10.0,0.0},{0.0,0.0,1.0});
	position_lines->add_line({0.0,0.0,0.0},
							{0.0,0.0,10.0},{0.0,1.0,0.0});

	//Adding a light
	light_1 = lights::simple_light::create_light(glm::vec3(1.0,1.0,1.0),
												 glm::vec3(1.0,1.0,1.0));
	LOG2("Entering main loop!");
	while(!glfwWindowShouldClose(window_ctx))
	{
		++current_fps;
		glfwPollEvents();
		evaluate_key_status();

		auto current_time = std::chrono::system_clock::now();
		if(std::chrono::duration_cast<
				std::chrono::milliseconds>(
					current_time - ref_time).count() > 1000){
			ref_time = current_time;
			fps_info->set_text(std::to_string(current_fps) + "fps");
			current_fps = 0;
		}

		glClearColor(0.5,0.5,0.5,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		position_lines->prepare_for_render();
		position_lines->render();
		position_lines->clean_after_render();

		object->set_transformations(model,camera->get_view(),projection);
		object->prepare_for_render();
		object->render();
		object->clean_after_render();

		light_1->set_transformation(model,camera->get_view(),projection);
		light_1->prepare_for_render();
		light_1->render();
		light_1->clear_after_render();

		fps_info->render_text();

		auto yaw = camera->get_camera_yaw(),
			pitch = camera->get_camera_pitch();
		auto pos = camera->get_camera_pos();

		std::stringstream ss;
		ss << "yaw:"<<yaw<<", pitch:"<<pitch<<". x:"<<pos.x<<",y:"<<pos.y<<",z:"<<pos.z;
		camera_info->set_text(ss.str());

		camera_info->render_text();

		glfwSwapBuffers(window_ctx);
	}
}

GLFWwindow *opengl_ui::get_win_ctx()
{
	return window_ctx;
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
