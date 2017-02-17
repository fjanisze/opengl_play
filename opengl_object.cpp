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
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,-1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
	};

	for(int i{0}; i < 36; ++i) {
		for(int j{0}; j<8 ; ++j) {
			*((GLfloat*)( &vertices[i] ) + j) = raw_vertices[i*8 + j];
		}
	}

	update_vertex_data();
}

texture_info little_object::load_texture(const std::string &filename,
								GLint wrapping_method)
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
	//glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D,0);
	check_for_errors();

	return TEX;
}

void little_object::update_vertex_data()
{
	for(int i{0};i<36;++i)
	{
		vertex_data[8*i    ] = vertices[i].x;
		vertex_data[8*i + 1] = vertices[i].y;
		vertex_data[8*i + 2] = vertices[i].z;
		vertex_data[8*i + 3] = vertices[i].t_x;
		vertex_data[8*i + 4] = vertices[i].t_y;
		vertex_data[8*i + 5] = vertices[i].n_x;
		vertex_data[8*i + 6] = vertices[i].n_y;
		vertex_data[8*i + 7] = vertices[i].n_z;
	}

}

bool little_object::any_object_selected()
{
	return sel_obj_it != objects.end();
}

void little_object::limit_render_distance(movable::mov_obj_ptr target,
								GLfloat radius)
{
	render_radius_origin = target;
	render_radius = radius;
}

glm::vec3 little_object::get_object_position()
{
	if( any_object_selected() ) {
		glm::vec4 pos = {1.0,1.0,1.0,1.0};
		pos  = sel_obj_it->second.model * pos;
		return glm::vec3{pos.x,pos.y,pos.z};
	}
}

void little_object::modify_view(glm::mat4 new_view)
{
	for( auto& object:objects ) {
		object.second.view = new_view;
	}
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
	elem.second.model = glm::scale( elem.second.model,
									glm::vec3(elem.second.scale,
											  elem.second.scale,
											  elem.second.scale) );
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
	elem.second.model = glm::translate(
				model,
				elem.second.position);
}

void little_object::object_rotation(rotation_axis axis,
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
		case mov_direction::right:
			amount *= -1;
		case mov_direction::left:
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
		sel_obj_it->second.scale = amount;
	} else {
		WARN1("scale: Select a valid object first!");
	}
}

void little_object::set_transformations(glm::mat4 view,
										glm::mat4 projection)
{
	for(auto& object:objects) {
		object.second.view = view;
		object.second.projection = projection;
	}
}

int little_object::add_object(const glm::vec3 &coordinates,
							  const glm::vec3& color,
							  GLfloat scale)
{
	int id = next_object_id++;
	object_data data;
	data.position = coordinates;
	data.color = color;
	data.scale = scale;
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

little_object::little_object() :
	lights::object_lighting(&obj_shader),
	selected_object{0},
	next_object_id{1},
	render_radius{ -1 } //No limits
{
	LOG1("little_object::little_object(): Construction.");

	obj_shader.load_vertex_shader(
				obj_shader.read_shader_body("../model_shader.vert"));
	obj_shader.load_fragment_shader(
				obj_shader.read_shader_body("../model_shader.frag"));

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
						  8 * sizeof(GLfloat),
						  (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,
						  8 * sizeof(GLfloat),
						  (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,
						  8 * sizeof(GLfloat),
						  (GLvoid*)(5*sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	textures.insert(std::make_pair("specular",
								   load_texture("../textures/container2_specular.png")));
	textures.insert(std::make_pair("container",
								   load_texture("../textures/container2.png")));

	check_for_errors();
	add_renderable(this);
}

little_object::~little_object()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
	for( auto& tex:textures ) {
		glDeleteTextures(1,&tex.second.texture);
	}
}

void little_object::prepare_for_render()
{
	//Load the shader and bind the VAO
	glBindTexture(GL_TEXTURE_2D, textures["container"]);
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
				  textures["specular"]);
	glUniform1i(glGetUniformLocation(obj_shader,
									 "loaded_texture_specular_map"),1);

	calculate_lighting();
}

void little_object::render()
{
	if( nullptr != render_radius_origin) {
		glm::vec3 render_origin = render_radius_origin->get_position();
		render_with_radius(render_origin);
	} else {
		render_all();
	}
}

/*
 * Those are mostly identical functions,
 * render_with_radius has some additional ifs,
 * to avoid those ifs at every frame for every
 * little_objects (there might be thousands of them)
 * I decided to introduce a little duplication
 * and have them divided. To avoid duplication I can use
 * one piece of code with one more if.
 * if( render_radius > 0 ) {
 *	if( distance(origin, position) > render_radius )
 *		skip rendering.
 * }
 * Little improvement, but better than nothing..
 */
void little_object::render_with_radius(glm::vec3 origin)
{
	glBindVertexArray(VAO);
	for(auto& object : objects) {
		if( glm::distance( origin, object.second.position ) > render_radius ) {
			continue;
		}
		apply_position(object);
		apply_transformations(object);

		apply_object_color(object.second.color);

		glDrawArrays(GL_TRIANGLES,0,36);
	}
	//Unbind the VAO
	glBindVertexArray(0);
}

void little_object::render_all()
{
	glBindVertexArray(VAO);
	for(auto& object : objects) {
		apply_position(object);
		apply_transformations(object);

		apply_object_color(object.second.color);

		glDrawArrays(GL_TRIANGLES,0,36);
	}
	//Unbind the VAO
	glBindVertexArray(0);
}

void little_object::clean_after_render()
{

}

}
