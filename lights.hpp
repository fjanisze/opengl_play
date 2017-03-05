#include <headers.hpp>
#include <shaders.hpp>
#include <renderable_object.hpp>
#include <movable_object.hpp>
#include "my_camera.hpp"

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
	Spot_light,
	Flash_light
};

class generic_light;

template<typename T>
using light_ptr = std::shared_ptr<T>;

using generic_light_ptr = light_ptr<generic_light>;

class object_lighting
{
	static std::vector<generic_light_ptr> all_lights;
	shaders::my_small_shaders * frag_shader;
	std::vector<GLfloat> light_data_buffer;
public:
	object_lighting(shaders::my_small_shaders* shader);
	void calculate_lighting();
	void apply_object_color(const glm::vec3& color);
	/*
	 * At the moment the limit on the amount of lights
	 * is relaxed, increasing the light_data_buffer is
	 * sufficient to increase the amount of lights.
	 * That's why those functions are returning always
	 * true, that might change in the future
	 */
	static bool can_add_simple_light() {
		return true;
	}
	template<typename LightT>
	static bool add_simple_light(generic_light_ptr new_light) {
		all_lights.emplace_back(new_light);
		return true;
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
		LOG1("Creating a new light, type: ",
			 light->light_type());
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
	/*
	 * Certain light data like position, color &c
	 * are commong whithin all the lights,
	 * this function fill those common information
	 */
	std::size_t fill_common_light_data();
public:
	generic_light();
	generic_light(glm::vec3 position,
				 glm::vec3 color,
				 GLfloat   strength);
	virtual type_of_light light_type() = 0;
	virtual ~generic_light();
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
	virtual const std::vector<GLfloat> &get_light_data();
	/*
	 * Each light might have a different data
	 * size since the amount of specific information
	 * might be different
	 */
	virtual std::size_t light_data_size();
	/*
	 * Attach the spot_light to the object,
	 * will cast the light in front of this object (+Z)
	 */
	virtual void attach_to_object( movable::mov_obj_ptr object );
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
	directional_light() = default;
	directional_light(glm::vec3 direction,
				 glm::vec3 color,
				 GLfloat   strength);

	type_of_light light_type(){
		return type_of_light::Directional_Light;
	}
	std::string renderable_nice_name() {
		return "directional_light";
	}
};

/*
 * Cast light only in a certain area or direction,
 * everything that is outside the radius of the spotlight
 * is not illuminated
 */
class spot_light : public generic_light
{
protected:
	GLfloat cut_off,
			out_cutoff;
	glm::vec3 light_direction;
	movable::mov_obj_ptr target_obj;
public:
	spot_light() = default;
	spot_light(glm::vec3 position,
			glm::vec3 color,
			GLfloat   strength,
			glm::vec3 direction,
			GLfloat cut_off_angle,
			GLfloat out_cutoff_angle);

	type_of_light light_type(){
		return type_of_light::Spot_light;
	}
	std::string renderable_nice_name() {
		return "spot_light";
	}

	void attach_to_object( movable::mov_obj_ptr object );

	std::size_t light_data_size() override;
	const std::vector<GLfloat>& get_light_data() override;
private:
	/*
	 * If we're attached to a movable object,
	 * then we need to recalculate the direction light
	 * if the target moves
	 */
	void recalculate_light_direction();
};

/*
 * A flashlight is a regular spot_light which is
 * 'attached' to the camera, it's position and
 * direction change at every camera movement
 */
class flash_light : public spot_light
{
	opengl_play::camera_obj camera_ptr;
public:
	flash_light() = default;
	flash_light(opengl_play::camera_obj camera,
			glm::vec3 color,
			GLfloat   strength,
			GLfloat cut_off_angle,
			GLfloat out_cutoff_angle);

	type_of_light light_type(){
		return type_of_light::Flash_light;
	}
	std::string renderable_nice_name() {
		return "flash_light";
	}

	const std::vector<GLfloat>& get_light_data() override;
};


}

#endif
