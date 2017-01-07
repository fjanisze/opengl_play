#include "lights.hpp"

namespace lights
{

std::vector<simple_light_ptr> simple_light::all_lights;

object_lighting::object_lighting(shaders::my_small_shaders * shader) :
	frag_shader{ shader },
	ambient_light_strenght{ 0 },
	ambient_light_color{ glm::vec3(0.0,0.0,0.0) }
{
	LOG1("New object_lighting");
}

void object_lighting::update_ambient_colors()
{
	GLint light_color_uniform = glGetUniformLocation(*frag_shader,
											   "ambient_light_color");
	glUniform3f(light_color_uniform,
				ambient_light_color.r,
				ambient_light_color.g,
				ambient_light_color.b);
	GLint light_strenght = glGetUniformLocation(*frag_shader,
											   "ambient_light_strenght");
	glUniform1f(light_strenght,
				ambient_light_strenght);
}

void object_lighting::calculate_lighting()
{
	static int supported_lights = 2;
	auto all_lights = simple_light::get_lights();
	ambient_light_color = glm::vec3(0.0,0.0,0.0);
	int light_cnt = 0;
	GLfloat light_pos[3 * supported_lights],
			light_color[3 * supported_lights],
			light_strength[2];
	for(auto & light : all_lights) {
		auto light_data = light->get_light_color();
		glm::vec3 cur_light = light_data.first;
		ambient_light_color.r = std::max(ambient_light_color.r,cur_light.r);
		ambient_light_color.g = std::max(ambient_light_color.g,cur_light.g);
		ambient_light_color.b = std::max(ambient_light_color.b,cur_light.b);
		auto pos = light->get_light_position();
		light_pos[3 * light_cnt + 0] = pos.x;
		light_pos[3 * light_cnt + 1] = pos.y;
		light_pos[3 * light_cnt + 2] = pos.z;

		light_color[3 * light_cnt + 0] = cur_light.r;
		light_color[3 * light_cnt + 1] = cur_light.g;
		light_color[3 * light_cnt + 2] = cur_light.b;

		light_strength[light_cnt] = light_data.second;
		++light_cnt;
	}
	GLint lp = glGetUniformLocation(*frag_shader,
							   "light_pos");
	GLint lc = glGetUniformLocation(*frag_shader,
							   "light_color");
	GLint ls = glGetUniformLocation(*frag_shader,
							   "light_strength");
	glUniform3fv(lp,light_cnt,light_pos);
	glUniform3fv(lc,light_cnt,light_color);
	glUniform1fv(ls,light_cnt,light_strength);
	//Load the ambient light
	update_ambient_colors();
}

void object_lighting::apply_object_color(const glm::vec3 &color)
{
	//Apply the object color
	GLint obj_color_uniform = glGetUniformLocation(*frag_shader,
											   "object_color");
	glUniform3f(obj_color_uniform,
				color.r,
				color.b,
				color.g);
}

simple_light::simple_light(glm::vec3 position,
						   glm::vec3 color,
						   GLfloat strenght) :
	light_position{ position },
	light_color{ color },
	color_strenght{ strenght }
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

	add_renderable(this);
}

simple_light::~simple_light()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
}

void simple_light::set_transformations(glm::mat4 m,glm::mat4 v,glm::mat4 p)
{
	model = m;
	view = v;
	projection = p;
}

GLfloat simple_light::get_strenght()
{
	return color_strenght;
}

void simple_light::set_strenght(GLfloat strenght)
{
	color_strenght = strenght;
}

std::pair<glm::vec3, GLfloat> simple_light::get_light_color()
{
	return std::make_pair(light_color,color_strenght);
}

glm::vec3 simple_light::get_light_position()
{
	return light_position;
}

void simple_light::set_light_position(const glm::vec3 &new_pos)
{
	light_position = new_pos;
}

void simple_light::prepare_for_render()
{
	light_shader.use_shaders();

	GLint obj_color_uniform = glGetUniformLocation(light_shader,
											   "light_color");
	glUniform3f(obj_color_uniform,
				light_color.r,
				light_color.g,
				light_color.b);

	GLint obj_col_strenght_uniform = glGetUniformLocation(light_shader,
											   "light_strenght");
	glUniform1f(obj_col_strenght_uniform,
				color_strenght);


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

void simple_light::clean_after_render()
{

}

}
