#include <opengl_object.hpp>

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


void little_object::init_vertices()
{
	GLfloat raw_vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	for(int i{0}; i < 36; ++i) {
		for(int j{0}; j<5 ; ++j) {
			*((GLfloat*)( &vertices[i] ) + j) = raw_vertices[i*5 + j];
		}
	}

	update_vertex_data();
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

void little_object::update_vertex_data()
{
	for(int i{0};i<36;++i)
	{
		vertex_data[5*i    ] = vertices[i].x;
		vertex_data[5*i + 1] = vertices[i].y;
		vertex_data[5*i + 2] = vertices[i].z;
		vertex_data[5*i + 3] = vertices[i].t_x;
		vertex_data[5*i + 4] = vertices[i].t_y;
	}

}

bool little_object::any_object_selected()
{
	return sel_obj_it != objects.end();
}

std::set<int> little_object::get_all_objects()
{
	std::set<int> ids;
	for(auto& elem:objects) {
		ids.insert(elem.first);
	}
	return ids;
}

void little_object::apply_transformations(decltype(objects)::value_type& elem)
{
	GLint model_loc = glGetUniformLocation(obj_shader,"model");
	GLint view_loc = glGetUniformLocation(obj_shader,"view");
	GLint projection_loc = glGetUniformLocation(obj_shader,"projection");

	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(elem.second.model));
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(elem.second.view));
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(elem.second.projection));
}

void little_object::apply_position(decltype(objects)::value_type& elem)
{
	glm::mat4 model;
	model = glm::translate(
					model,
					elem.second.position);
	glUniformMatrix4fv(glGetUniformLocation(obj_shader,
								"object_position"),
								1, GL_FALSE,
								glm::value_ptr(model));
}

void little_object::image_rotation(rotation_axis axis,
								   GLfloat amount)
{
	if( any_object_selected() ) {
		static glm::vec3 rot_axis_defs[3]{
			glm::vec3(1.0,0.0,0.0),
			glm::vec3(0.0,1.0,0.0),
			glm::vec3(0.0,0.0,1.0),
		};
		sel_obj_it->second.model = glm::rotate(
					sel_obj_it->second.model,
					amount,
					rot_axis_defs[static_cast<int>(axis)]);
	} else {
		WARN1("image_rotation: Select a valid object first!");
	}
}

void little_object::move(mov_direction dir, GLfloat amount)
{
	if( any_object_selected() ) {
		glm::vec4 translation_vector;

		switch(dir) {
		case mov_direction::down:
			amount *= -1;
		case mov_direction::top:
			translation_vector.y = amount;
			break;
		case mov_direction::left:
			amount *= -1;
		case mov_direction::right:
			translation_vector.x = amount;
			break;
		}

		sel_obj_it->second.model = glm::translate(
						sel_obj_it->second.model,
						glm::vec3(translation_vector.x,
								  translation_vector.y,
								  translation_vector.z));
	} else {
		WARN1("move: Select a valid object first!");
	}
}

void little_object::scale(GLfloat amount)
{
	if( any_object_selected() ) {
		sel_obj_it->second.model = glm::scale(
					sel_obj_it->second.model,
					glm::vec3(amount,amount,amount));
	} else {
		WARN1("scale: Select a valid object first!");
	}
}

void little_object::set_transformations(glm::mat4 model,
										glm::mat4 view,
										glm::mat4 projection)
{
	if( any_object_selected() ) {
		sel_obj_it->second.model = model;
		sel_obj_it->second.view = view;
		sel_obj_it->second.projection = projection;
	} else {
		WARN1("set_transformations: Select a valid object first!");
	}
}

int little_object::add_object(const glm::vec3 &coordinates)
{
	int id = next_object_id++;
	object_data data;
	data.position = coordinates;
	objects.insert({id, data});
	return id;
}

bool little_object::select_object(int id)
{
	auto it = objects.find(id);
	if( it != objects.end() ) {
		selected_object = id;
		sel_obj_it = it;
		return true;
	}
	return false;
}

bool little_object::release_current_object()
{
	selected_object = 0;
	sel_obj_it = objects.end();
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
	current_mix_ratio{0.2},
	selected_object{0},
	next_object_id{1}
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

	//Save this condiguration in VAO.
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(vertex_data),
				 vertex_data,
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
						  5 * sizeof(GLfloat),
						  (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,
						  5 * sizeof(GLfloat),
						  (GLvoid*)(3*sizeof(GLfloat)));
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

}

void little_object::render()
{
	glBindVertexArray(VAO);
	for(auto& object : objects) {
		apply_position(object);
		apply_transformations(object);
		glDrawArrays(GL_TRIANGLES,0,36);
	}
	//Unbind the VAO
	glBindVertexArray(0);
}

void little_object::clean_after_render()
{

}

}
