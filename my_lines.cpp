#include <my_lines.hpp>

namespace opengl_play
{

my_static_lines::my_static_lines()
{
	LOG1("my_lines::my_lines(): Construction");
	shaders.load_vertex_shader(
				shaders.read_shader_body("../line_shader.vert"));
	shaders.load_fragment_shader(
				shaders.read_shader_body("../line_shader.frag"));

	if(!shaders.create_shader_program()){
		ERR("Unable to create the shader program!");
		throw std::runtime_error("Shader program creation failure!");
	}

	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);

	add_renderable(this);
}

my_static_lines::~my_static_lines()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
}

int my_static_lines::add_line(glm::vec3 from, glm::vec3 to,
					glm::vec3 color)
{
	single_line new_line = {
		{from.x,from.y,from.z},
		{color.r,color.b,color.g},
		{to.x,to.y,to.z},
		{color.r,color.b,color.g}
	};
	lines.push_back(new_line);
	update_buffers();
	return lines.size();
}

void my_static_lines::set_transformations(glm::mat4 v,
								glm::mat4 p)
{
	view = v;
	projection = p;
}

void my_static_lines::update_buffers()
{
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,
				 lines.size() * sizeof(single_line),
				 lines.data(),
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
						  6 * sizeof(GLfloat),
						  (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
						  6 * sizeof(GLfloat),
						  (GLvoid*)(3* sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
}

void my_static_lines::modify_view(glm::mat4 new_view)
{
	view = new_view;
}

void my_static_lines::prepare_for_render()
{
	shaders.use_shaders();
	GLint view_loc = glGetUniformLocation(shaders,"view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
	GLint projection_loc = glGetUniformLocation(shaders,"projection");
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 model;
	GLint model_loc = glGetUniformLocation(shaders,"model");
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

}

void my_static_lines::render()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP,0,lines.size() * 2);
	glBindVertexArray(0);
}

void my_static_lines::clean_after_render()
{

}

}
