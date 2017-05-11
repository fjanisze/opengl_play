#include <headers.hpp>
#include <shaders.hpp>
#include <movable_object.hpp>
#include "my_camera.hpp"

#ifndef LIGHTS_HPP
#define LIGHTS_HPP

namespace lighting
{

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

class Generic_light;

using light_ptr = std::shared_ptr< Generic_light >;

class Core_lighting
{
public:
    Core_lighting();
    void calculate_lighting(shaders::Shader::pointer &shader );
    void add_light( light_ptr obj );
private:
    std::vector< light_ptr > lights;
    std::vector< GLfloat > light_data_buffer;
};

using lighting_pointer = std::shared_ptr< Core_lighting >;

class Generic_light : public movable::movable_object
{
public:
    Generic_light();
    Generic_light(glm::vec3 position,
                  glm::vec4 color,
                  GLfloat   strength);
    virtual type_of_light light_type() = 0;
    GLfloat get_strength();
    void    set_strength(GLfloat strength);
    std::pair<glm::vec4,GLfloat> get_light_color();
    void set_light_color( const glm::vec4& new_color );
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
    virtual ~Generic_light();
protected:
    glm::vec4 light_color;
    GLfloat   color_strength;
    std::vector<GLfloat> light_data;
    /*
     * Certain light data like position, color &c
     * are commong whithin all the lights,
     * this function fill those common information
     */
    std::size_t fill_common_light_data();
};

/*
 * Cast light in every direction at equal
 * intensity
 */
class point_light : public Generic_light
{
public:
    point_light() = default;
    point_light(glm::vec3 position,
                glm::vec4 color,
                GLfloat   strength);
    type_of_light light_type() {
        return type_of_light::Point_Light;
    }
    ~point_light();
};

/*
 * Directional lights do not have a position,
 * they are somewhere far away and the rays are
 * coming from a certain 'direction' toward
 * out scene.
 */
class directional_light : public Generic_light
{
public:
    directional_light() = default;
    directional_light(glm::vec3 direction,
                      glm::vec4 color,
                      GLfloat   strength);

    type_of_light light_type(){
        return type_of_light::Directional_Light;
    }
};

/*
 * Cast light only in a certain area or direction,
 * everything that is outside the radius of the spotlight
 * is not illuminated
 */
class spot_light : public Generic_light
{
protected:
    GLfloat cut_off,
    out_cutoff;
    glm::vec3 light_direction;
    movable::mov_obj_ptr target_obj;
public:
    spot_light() = default;
    spot_light(glm::vec3 position,
               glm::vec4 color,
               GLfloat   strength,
               glm::vec3 direction,
               GLfloat cut_off_angle,
               GLfloat out_cutoff_angle);

    type_of_light light_type(){
        return type_of_light::Spot_light;
    }

    std::size_t light_data_size() override;
    const std::vector<GLfloat>& get_light_data() override;
};

/*
 * A flashlight is a regular spot_light which is
 * 'attached' to the camera, it's position and
 * direction change at every camera movement
 */
class flash_light : public spot_light
{
    opengl_play::camera_ptr camera_ptr;
public:
    flash_light() = default;
    flash_light(opengl_play::camera_ptr camera,
                glm::vec4 color,
                GLfloat   strength,
                GLfloat cut_off_angle,
                GLfloat out_cutoff_angle);

    type_of_light light_type(){
        return type_of_light::Flash_light;
    }

    const std::vector<GLfloat>& get_light_data() override;
};

template<typename LightT>
struct Light_factory
{
    template<typename...Args>
    static light_ptr create(Args&&...args) {
        return std::make_shared<  LightT>(std::forward<Args>(args)...);
    }
};

}

#endif
