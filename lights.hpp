#include <headers.hpp>
#include <shaders.hpp>
#include <renderable_object.hpp>
#include <movable_object.hpp>

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

/*
 * Type of available lights,
 * this enum is used to map the light
 * data with the proper light function
 * in the shader
 */
enum type_of_light {
	Point_Light,
	Directional_Light,
	Spot_light
};

class generic_light;

template<typename T>
using light_ptr = std::shared_ptr<T>;

using generic_light_ptr = light_ptr<generic_light>;

class object_lighting
{
	static std::vector<generic_light_ptr> all_lights;
	shaders::my_small_shaders * frag_shader;
	void      update_ambient_colors();
	GLfloat   ambient_light_strength;
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

class generic_light : public renderable::renderable_object,
		public movable::movable_object
{
protected:
	GLuint VAO,VBO;
	shaders::my_small_shaders light_shader;
	std::unique_ptr<GLfloat[]> cube_vrtx;
	glm::vec3 light_color;
	GLfloat   color_strength;
	glm::mat4 view,projection;
	std::vector<GLfloat> light_data;
	virtual void init_render_buffers() throw (std::runtime_error);
public:
	generic_light() = default;
	generic_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strength);
	virtual type_of_light light_type() = 0;
	~generic_light();
	GLfloat get_strength();
	void    set_strength(GLfloat strength);
	std::pair<glm::vec3,GLfloat> get_light_color();
	/*
	 * get_light_data return a vector with
	 * all the information needed to manipulate/render
	 * the light. Those informations are processed
	 * by the fragment shader.
	 *
	 * What does each light return in this vector
	 * depends on the light itself, but mostly contains
	 * stuff like: type,position,color &c.
	 */
	std::vector<GLfloat> get_light_data();
};

/*
 * Cast light in every direction at equal
 * intensity
 */
class point_light : public generic_light
{
public:
	point_light() = default;
	point_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strength);
	type_of_light light_type() {
		return type_of_light::Point_Light;
	}
	~point_light();
	void set_transformations(glm::mat4 v, glm::mat4 p);
	void prepare_for_render();
	void render();
	void clean_after_render();

	void rotate_object(GLfloat yaw);
};

/*
 * Directional lights do not have a position,
 * they are somewhere far away and the rays are
 * coming from a certain 'direction' toward
 * out scene.
 */
class directional_light : public generic_light
{
public:
	directional_light(glm::vec3 direction,
				 glm::vec3 color,
				 GLfloat   strength);

	type_of_light light_type(){
		return type_of_light::Directional_Light;
	}

	directional_light() {}
};

/*
 * Cast light only in a certain area or direction,
 * everything that is outside the radius of the spotlight
 * is not illuminated
 */
class spot_light : public generic_light
{
public:
	spot_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strength);

	type_of_light light_type(){
		return type_of_light::Spot_light;
	}

	spot_light() {}
};


}

#endif
