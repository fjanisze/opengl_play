#include "lights.hpp"

namespace lights
{

simple_light::simple_light(glm::vec3 position,
						   glm::vec3 color) :
	light_position{ position },
	light_color{ color }
{
	LOG1("Creating new simple_light!");

	cube_vrtx = std::make_unique<GLfloat[]>(36 * 3);
	std::copy(model_vertices::cube_vertices,
			  model_vertices::cube_vertices + 36 * 3 - 1,
			  cube_vrtx.get());

	light_shader.load_vertex_shader(
				light_shader.read_shader_body("../light_shader.vert"));
	light_shader.load_fragment_shader(
				light_shader.read_shader_body("../light_shader.frag"));

	if(!light_shader.create_shader_program()) {
		ERR("Unable to create the shader program");
		throw std::runtime_error("Failed to create simple_light");
	}

	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(GLfloat) * 36 * 3,
				 cube_vrtx.get(),
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
						  sizeof(GL_FLOAT) * 3,
						  (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
}

simple_light::~simple_light()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
}

void simple_light::set_transformation(glm::mat4 m,glm::mat4 v,glm::mat4 p)
{
	model = m;
	view = v;
	projection = p;
}

void simple_light::prepare_for_render()
{
	light_shader.use_shaders();

	glm::mat4 model;
	model = glm::translate(
					model,
					light_position);
	model = glm::scale(model,
					   glm::vec3(0.5,0.5,0.5));
	glUniformMatrix4fv(glGetUniformLocation(light_shader,
								"object_position"),
								1, GL_FALSE,
								glm::value_ptr(model));

	GLint model_loc = glGetUniformLocation(light_shader,"model");
	GLint view_loc = glGetUniformLocation(light_shader,"view");
	GLint projection_loc = glGetUniformLocation(light_shader,"projection");

	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
}

void simple_light::render()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES,0,36);
	glBindVertexArray(0);
}

void simple_light::clear_after_render()
{

}

}
