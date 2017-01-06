#include <headers.hpp>
#include <shaders.hpp>
#include <renderable_object.hpp>

#ifndef LIGHTS_HPP
#define LIGHTS_HPP

namespace lights
{

namespace model_vertices
{
const GLfloat cube_vertices[] = {
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
};
}

class simple_light;

using simple_light_ptr = std::shared_ptr<simple_light>;

class simple_light : renderable::renderable_object
{
	GLuint VAO,VBO;
	shaders::my_small_shaders light_shader;
	std::unique_ptr<GLfloat[]> cube_vrtx;
	glm::vec3 light_position,
			  light_color;
	GLfloat   color_strenght;
	glm::mat4 model,view,projection;
	static std::vector<simple_light_ptr> all_lights;
public:
	simple_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strenght);
	~simple_light();
	void set_transformations(glm::mat4 m, glm::mat4 v, glm::mat4 p);
	GLfloat get_strenght();
	void    set_strenght(GLfloat strenght);
	std::pair<glm::vec3,GLfloat> get_light_color();
	glm::vec3 get_light_position();
	void      set_light_position(const glm::vec3& new_pos);
	void prepare_for_render();
	void render();
	void clean_after_render();
	template<typename...Args>
	static simple_light_ptr create_light(Args&&...args)
	{
		auto new_light = std::make_shared<simple_light>(std::forward<Args>(args)...);
		all_lights.push_back(new_light);
		return new_light;
	}
	static auto get_lights() -> std::add_lvalue_reference<decltype(all_lights)>::type {
		return all_lights;
	}
};

class object_lighting
{
	shaders::my_small_shaders * frag_shader;
	void      update_ambient_colors();
	GLfloat   ambient_light_strenght;
	glm::vec3 ambient_light_color;
public:
	object_lighting(shaders::my_small_shaders* shader);
	void calculate_lighting();
	void apply_object_color(const glm::vec3& color);
	virtual ~object_lighting() {}
};

}

#endif
