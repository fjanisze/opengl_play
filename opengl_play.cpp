#include "opengl_play.hpp"
#include <models.hpp>
#include <iomanip>

namespace opengl_play
{

namespace
{
//Saved context for the GLFW callbacks
opengl_ui* ui_instance;
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
	if( button == GLFW_MOUSE_BUTTON_LEFT &&
		action == GLFW_PRESS ) {
		//Create a new light (The positioning do not work properly)
		glm::vec3 light_pos = camera->get_position();

		LOG1("Creating a new light at pos: ",
			 light_pos.x,"/",light_pos.y,"/",light_pos.z);
		auto light = lights::light_factory<lights::point_light>::create(
					light_pos,
					glm::vec3(1.0,1.0,1.0),
					6.0);
		if( nullptr == light ) {
			WARN1("Unable to create a new light!");
		}
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

	last_mouse_x = x;
	last_mouse_y = y;
	movement_processor.mouse_input(x, y);
}

void opengl_ui::ui_keyboard_press(GLint button,
								  GLint scode,
								  GLint action)
{
	if( button < 0 ) {
		//Not recognized button
		return;
	}
	if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
		if( button == GLFW_KEY_ESCAPE ) {
			glfwSetWindowShouldClose(window_ctx,1);
		} else {
			key_status[ button ] = key_status_t::pressed;
		}
	} else if( action == GLFW_RELEASE ) {
		key_status[ button ] = key_status_t::not_pressed;
	}
	movement_processor.keyboard_input(button,scode,action);
}

void opengl_ui::evaluate_key_status()
{
}

void opengl_ui::setup_scene()
{
	const std::pair<glm::vec3,glm::vec3> line_endpoints[] = {
		{{50,0,0},{1.0,0.0,0.0}},
		{{-50,0,0},{1.0,0.0,0.0}},
		{{0,50,0},{0.0,0.0,1.0}},
		{{0,-50,0},{0.0,0.0,1.0}},
		{{0,0,50},{0.0,1.0,0.0}},
		{{0,0,-50},{0.0,1.0,0.0}},
		{{50,50,50},{0.0,1.0,1.0}},
		{{-50,-50,-50},{0.0,1.0,1.0}}
	};

	for(auto& elem : line_endpoints) {
		position_lines->add_line({0.0,0.0,0.0},elem.first,elem.second);
	}

	//Let our model be movable
	//Register the camera as movable object
	movable::key_mapping_vec camera_keys = {
		{ GLFW_KEY_W, { movable::mov_direction::top, { 0.7 } } },
		{ GLFW_KEY_S, { movable::mov_direction::down, { 0.3 } } },
		{ GLFW_KEY_A, { movable::mov_direction::left, { 0.3 } } },
		{ GLFW_KEY_D, { movable::mov_direction::right, { 0.3 } } },
	};

	movable::mouse_mapping_vec camera_mouse = {
		{ movable::mouse_movement_types::pitch_increse, { movable::mov_direction::pitch_inc, { 0.05 } } },
		{ movable::mouse_movement_types::pitch_decrease, { movable::mov_direction::pitch_dec, { 0.05 } } },
		{ movable::mouse_movement_types::yaw_increase, { movable::mov_direction::yaw_dec, { 0.05 } } },
		{ movable::mouse_movement_types::yaw_decrease, { movable::mov_direction::yaw_inc, { 0.05 } } },
	};


	movement_processor.register_movable_object(camera,camera_keys);
	movement_processor.register_movable_object(camera,camera_mouse);
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
	fps_info->set_scale(0.4f);
	fps_info->set_text("0 fps");
	fps_info->set_window_size(win_h,win_w);

	camera_info = std::make_shared<text_renderer::renderable_text>();
	camera_info->set_position(glm::fvec2(win_h - 300,7));
	camera_info->set_color(glm::vec3(1.0f,1.0f,1.0f));
	camera_info->set_scale(0.4f);
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

	camera = my_camera::create_camera({10.0,10.0,10.0},{0.0,0.0,0.0});

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
	setup_scene();
	check_for_errors();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST | GL_STENCIL_TEST);
	glfwSetInputMode(window_ctx, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	auto ref_time = std::chrono::system_clock::now();
	int  current_fps = 0;

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f),
						(GLfloat)win_w / (GLfloat)win_h,
						10.0f, 20000.0f);


	LOG2("Entering main loop!");
	while(!glfwWindowShouldClose(window_ctx))
	{
		++current_fps;
		glfwPollEvents();
		evaluate_key_status();
		movement_processor.process_movements();
		camera->follow_target();

		auto current_time = std::chrono::system_clock::now();
		if(std::chrono::duration_cast<
				std::chrono::milliseconds>(
					current_time - ref_time).count() > 1000){
			ref_time = current_time;
			fps_info->set_text(std::to_string(current_fps) + "fps");
			current_fps = 0;
		}

		glClearColor(0.0,0.2,0.02,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		renderable::renderable_object::render_renderables(camera->get_view(),
											projection);

		fps_info->render_text();

		auto yaw = camera->get_yaw(),
			pitch = camera->get_pitch(),
			roll = camera->get_roll();
		auto pos = camera->get_position();

		std::stringstream ss;
		ss <<std::setprecision(2)<<std::fixed<< "yaw:"<<yaw<<", pitch:"<<pitch<<", roll:"<<roll
		   <<". x:"<<pos.x<<",y:"<<pos.y<<",z:"<<pos.z;

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
	opengl_play::opengl_ui entry(1920,1280);
	log_inst.set_thread_name("MAIN");

	entry.prepare_for_main_loop();
	entry.enter_main_loop();
}
