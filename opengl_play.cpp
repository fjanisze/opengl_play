#include "opengl_play.hpp"

namespace opengl_play
{

int check_for_errors()
{
	int error_count = 0;
	GLenum error = GL_NO_ERROR;

	while( (error = glGetError()) != GL_NO_ERROR){
		ERR("OpenGL ERROR in opengl_ui: ",
			error);
		++error_count;
	}
	return error_count;
}

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

void opengl_ui::setup_callbacks()
{

	glfwSetMouseButtonCallback(window_ctx,
							   mouse_click_callback);
	glfwSetCursorPosCallback(window_ctx,
							 cursor_pos_callback);
	glfwSetWindowSizeCallback(window_ctx,
							  window_resize_callback);
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

	LOG2("Entering main loop!");
	while(!glfwWindowShouldClose(window_ctx))
	{
		glfwPollEvents();

		//Do not draw anythin if is not needed
		if(false == render_update_needed)
		{
			continue;
		}

		glClearColor(0.5,0.5,0.5,1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		object->prepare_for_render();
		object->render();
		object->clean_after_render();

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

void opengl_ui::ui_mouse_click(GLint button, GLint action)
{
	object->mouse_click(button,action);
}

opengl_ui::~opengl_ui()
{
	if(window_ctx!=nullptr){
		glfwTerminate();
	}
}

void little_object::init_vertices()
{
	vertices[0] = { 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
	vertices[1] = { 0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	vertices[2] = {-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	vertices[3] = {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	for(int i{0};i<4;++i)
	{
		vertex_data[8*i    ] = vertices[i].x;
		vertex_data[8*i + 1] = vertices[i].y;
		vertex_data[8*i + 2] = vertices[i].z;
		vertex_data[8*i + 3] = vertices[i].r;
		vertex_data[8*i + 4] = vertices[i].g;
		vertex_data[8*i + 5] = vertices[i].b;
		vertex_data[8*i + 6] = vertices[i].t_x;
		vertex_data[8*i + 7] = vertices[i].t_y;
	}

	int i{0};
	for(auto idx:{0, 1, 3, 1, 2, 3})
		vertex_idxs[i++] = idx;
}

texture_info little_object::load_texture(const std::string &filename, GLint wrapping_method)
{
	LOG1("Creating the texture for little_object");
	texture_info TEX;
	//Load the texture image
	byte_t* image = SOIL_load_image(filename.c_str(),
									&TEX.width,
									&TEX.height,
									0,
									SOIL_LOAD_RGB);
	if(image == nullptr){
		ERR("Unable to load the texture!");
		throw std::runtime_error("Texture loading failed!");
	}
	LOG1("Loaded texture size: ",TEX.width,"/",TEX.height);
	//Create the texture
	glGenTextures(1,&TEX.texture);
	glBindTexture(GL_TEXTURE_2D,TEX.texture);

	//Texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_WRAP_S, wrapping_method);
	glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_WRAP_T, wrapping_method);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,
					GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,	0,
				 GL_RGB,
				 TEX.width,
				 TEX.height,
				 0,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 image);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D,0);
	check_for_errors();

	return TEX;
}

void little_object::mouse_click(GLint button, GLint action)
{
	if(action != GLFW_PRESS)
		return;
	if(button == GLFW_MOUSE_BUTTON_LEFT){
		current_mix_ratio = std::min(1.0,
									 current_mix_ratio + 0.05);
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		current_mix_ratio = std::max<GLfloat>(0,
											  current_mix_ratio - 0.05);
	}
}

little_object::little_object() :
	current_mix_ratio{0.2}
{
	LOG1("little_object::little_object(): Construction.");

	obj_shader.load_vertex_shader(
				obj_shader.read_shader_body("../shader_1.vert"));
	obj_shader.load_fragment_shader(
				obj_shader.read_shader_body("../shader_1.frag"));

	if(!obj_shader.create_shader_program()){
		ERR("Unable to create the shader program!");
		throw std::runtime_error("Shader program creation failure!");
	}

	init_vertices();

	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	glGenBuffers(1,&EBO);

	//Save this condiguration in VAO.
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(vertex_data),
				 vertex_data,
				 GL_STATIC_DRAW);

	//Setup the Element Buffer Object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(vertex_idxs),
				 vertex_idxs,
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
						  8 * sizeof(GLfloat),
						  (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,
						  8 * sizeof(GLfloat),
						  (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,
						  8 * sizeof(GLfloat),
						  (GLvoid*)(6*sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	textures.insert(std::make_pair("container",
								   load_texture("../textures/container.jpg")));
	textures.insert(std::make_pair("face",
								   load_texture("../textures/awesomeface.png")));

	check_for_errors();
}

little_object::~little_object()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
	glDeleteBuffers(1,&EBO);
}

void little_object::prepare_for_render()
{
	//Load the shader and bind the VAO
	glBindTexture(GL_TEXTURE_2D, textures["face"]);
	obj_shader.use_shaders();

	//Activate the two texture.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
				  textures["container"]);
	//This uniform is for the texture unit 0
	glUniform1i(glGetUniformLocation(obj_shader,
									 "loaded_texture"),0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,
				  textures["face"]);
	glUniform1i(glGetUniformLocation(obj_shader,
									 "loaded_texture_2"),1);

	//Setup the mix ratio
	glUniform1f(glGetUniformLocation(obj_shader,
									 "mix_ratio"),current_mix_ratio);

	glBindVertexArray(VAO);
}

void little_object::render()
{
	glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
}

void little_object::clean_after_render()
{
	//Unbind the VAO
	glBindVertexArray(0);
}

}

int main()
{
	opengl_play::opengl_ui entry(800,600);
	log_inst.set_thread_name("MAIN");

	entry.prepare_for_main_loop();
	entry.enter_main_loop();
}
