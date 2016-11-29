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
	vertices[0] = { 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
	vertices[1] = { 0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	vertices[2] = {-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	vertices[3] = {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	update_vertex_data();

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

void little_object::update_vertex_data()
{
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

}

void little_object::image_rotation(GLfloat amount)
{
	object_position = glm::rotate(
				object_position,
				amount,
				glm::vec3(0.0,0.0,1.0));
}

void little_object::move(mov_direction dir, GLfloat amount)
{
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

	object_position = glm::translate(
				object_position,
				glm::vec3(translation_vector.x,
						  translation_vector.y,
						  translation_vector.z));
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

	//Load the transformation matrix
	glUniformMatrix4fv(glGetUniformLocation(obj_shader,
								"transform_matrix"),
								1, GL_FALSE,
								glm::value_ptr(object_position));
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
