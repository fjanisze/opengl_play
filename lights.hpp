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

class generic_light;

template<typename T>
using light_ptr = std::shared_ptr<T>;

using generic_light_ptr = light_ptr<generic_light>;

class object_lighting
{
	static std::vector<generic_light_ptr> all_lights;
	shaders::my_small_shaders * frag_shader;
	void      update_ambient_colors();
	GLfloat   ambient_light_strenght;
	glm::vec3 ambient_light_color;
public:
	object_lighting(shaders::my_small_shaders* shader);
	void calculate_lighting();
	void apply_object_color(const glm::vec3& color);
	static bool can_add_simple_light() {
		return all_lights.size() < 10;
	}
	template<typename LightT>
	static bool add_simple_light(generic_light_ptr new_light) {
		if( all_lights.size() < 10 ) {
			all_lights.emplace_back(new_light);
			return true;
		}
		return false;
	}
	virtual ~object_lighting() {}
};

template<typename LightT>
struct light_factory
{
	template<typename...Args>
	static light_ptr<LightT> create(Args&&...args) {
		if( false == object_lighting::can_add_simple_light() ) {
			return nullptr;
		}
		auto light = std::make_shared<LightT>(std::forward<Args>(args)...);
		if( false == object_lighting::add_simple_light<LightT>(light) ) {
			return nullptr;
		}
		return light;
	}
};

class generic_light
{
protected:
	GLuint VAO,VBO;
	shaders::my_small_shaders light_shader;
	std::unique_ptr<GLfloat[]> cube_vrtx;
	glm::vec3 light_position,
			  light_color;
	GLfloat   color_strenght;
	glm::mat4 model,view,projection;
	virtual void init_render_buffers() throw (std::runtime_error);
public:
	generic_light() = default;
	generic_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strenght);
	~generic_light();
	GLfloat get_strenght();
	void    set_strenght(GLfloat strenght);
	std::pair<glm::vec3,GLfloat> get_light_color();
	glm::vec3 get_light_position();
	void set_light_position(const glm::vec3& new_pos);
};

class point_light : public generic_light,
		public renderable::renderable_object
{
public:
	point_light() = default;
	point_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strenght);
	~point_light();
	void set_transformations(glm::mat4 m, glm::mat4 v, glm::mat4 p);
	void prepare_for_render();
	void render();
	void clean_after_render();
};

class directional_light : public generic_light
{
	/*
	 * Directional lights do not have a position,
	 * they are somewhere far away and the rays are
	 * coming from a certain 'direction' toward
	 * out scene.
	 */
	glm::vec3 light_direction;
public:
	directional_light(glm::vec3 direction,
				 glm::vec3 color,
				 GLfloat   strenght);
};


}

#endif
