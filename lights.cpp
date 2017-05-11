#include "lights.hpp"

namespace lighting
{

Core_lighting::Core_lighting()
{
    LOG3("New Core_lighting");
    /*
     * Hardcoded size! This allow to support
     * something like 128 lights!
     */
    light_data_buffer.resize( 1024 ); //TODO, there should be one buffer for ALL, not many copies with identical data (see calculate_lighting)
    LOG1("Data buffer size: ", light_data_buffer.size() );
}

/*
 * Calculate the ambient light color and intensity,
 * load to the proper uniforms the position,color and strength
 * of all the lights in the scene.
 */
void Core_lighting::calculate_lighting( shaders::Shader::pointer& shader )
{
    int light_cnt = 0;
    std::size_t current_idx{ 0 };

    for(auto & light : lights) {
        auto light_data = light->get_light_data();
        for( auto& entry : light_data ) {
            light_data_buffer[ current_idx++ ] = entry;
            if( current_idx >= 1024 ) {
                ERR("Too much light data to fit!");
                throw std::runtime_error("light_data_buffer too small!!");
            }
        }
        ++light_cnt;
    }

    GLint nl = glGetUniformLocation(shader->get_program(),
                                    "number_of_lights");

    if( nl < 0 )
    {
        ERR("Unable to load the uniform number_of_lights");
    }
    else
    {
        glUniform1i(nl,lights.size());
    }

    GLint shader_light_buffer = glGetUniformLocation(shader->get_program(),
                                                     "light_data");

    if( shader_light_buffer < 0 )
    {
        ERR("Unable to load the uniform light_data");
    }
    else
    {
        glUniform1fv(shader_light_buffer,
                     current_idx,
                     light_data_buffer.data());
    }
}

void Core_lighting::add_light( light_ptr obj )
{
    LOG3("Adding new light");
    lights.push_back( obj );
    LOG3("Amount of lights: ", lights.size());
}


//////////////////////////////////////
/// generic_light implementation
/////////////////////////////////////

std::size_t Generic_light::fill_common_light_data()
{
    light_data[0] = static_cast<int>(light_type());
    glm::vec3 pos = get_position();
    light_data[1] = pos.x;
    light_data[2] = pos.y;
    light_data[3] = pos.z;
    auto color_info = get_light_color();
    light_data[4] = color_info.first.r;
    light_data[5] = color_info.first.g;
    light_data[6] = color_info.first.b;
    light_data[7] = color_info.first.a;
    light_data[8] = color_info.second;
    return 9; //Next valid index
}

Generic_light::Generic_light()
{
    //Setup the default size for a generic light
    light_data.resize(light_data_size());
}

Generic_light::Generic_light(glm::vec3 position,
                             glm::vec4 color,
                             GLfloat strength) :
    light_color{ color },
    color_strength{ strength }
{
    set_position(position);
    set_scale(0.5);
    //Setup the default size for a generic light
    light_data.resize(light_data_size());
}

Generic_light::~Generic_light()
{
}

GLfloat Generic_light::get_strength()
{
    return color_strength;
}

void Generic_light::set_strength(GLfloat strength)
{
    color_strength = strength;
}

std::pair<glm::vec4, GLfloat> Generic_light::get_light_color()
{
    return std::make_pair(light_color,color_strength);
}

void Generic_light::set_light_color(const glm::vec4 &new_color)
{
    light_color = new_color;
}

std::size_t Generic_light::light_data_size()
{
    /*
     * Those are the default values for a generic light
     */
    return 1 + //type
            3 + //position
            4 + //color
            1;  //strength
}

const std::vector<GLfloat>& Generic_light::get_light_data()
{
    /*
     * The format of the data for a generic
     * light is :
     * light_type = 1 float (0)
     * light_pos  = 3 float (x,y,z)
     * light_color= 3 float (r,b,g)
     * strength   = 1 float (s)
     */
    //TODO: Do not always update, just if needed
    fill_common_light_data();
    return light_data;
}


//////////////////////////////////////
/// point_light implementation
/////////////////////////////////////


point_light::point_light(glm::vec3 position,
                         glm::vec4 color,
                         GLfloat strength) :
    Generic_light(position,color,strength)
{
    LOG1("New point_light");
}

point_light::~point_light()
{
}

//////////////////////////////////////
/// directional_light implementation
/////////////////////////////////////

directional_light::directional_light(glm::vec3 direction,
                                     glm::vec4 color,
                                     GLfloat strength)
{
    LOG3("Creating a new directional light!");
    set_position(direction);
    light_color = color;
    color_strength = strength;
}

//////////////////////////////////////
/// spot_light and flash_light implementation
/////////////////////////////////////

/*
 * In the constructor the parameter 'direction'
 * is supposed to be the point where the light
 * is pointing, this point is understand by the
 * shader as the offset from the current light
 * position, so to make the whole thing work
 * correctly we need to calculate light_direction
 * as the offset 'from' the current position TO
 * the 'direction' choosen by the user.
 */
spot_light::spot_light(glm::vec3 position,
                       glm::vec4 color,
                       GLfloat strength,
                       glm::vec3 direction,
                       GLfloat cut_off_angle,
                       GLfloat out_cutoff_angle) :
    Generic_light::Generic_light(position,color,strength),
    light_direction{ direction - position },
    cut_off{ glm::cos( glm::radians(cut_off_angle) ) },
    out_cutoff{ glm::cos( glm::radians( out_cutoff_angle) ) }
{
    LOG3("Creating a new spot_light");
    light_data.resize( light_data_size() );
}

std::size_t spot_light::light_data_size()
{
    /*
     * The additional elements are the light direction
     * plus the two cutoff angles
     */
    return Generic_light::light_data_size() + 5;
}

const std::vector<GLfloat> &spot_light::get_light_data()
{
    std::size_t idx{ 0 };
    if( target_obj == nullptr ) {
        idx = fill_common_light_data();
        light_data[ idx     ] = light_direction.x;
        light_data[ idx + 1 ] = light_direction.y;
        light_data[ idx + 2 ] = light_direction.z;
        idx += 3;
    } else {
        //Calculate the new position and direction
        //on the base of the target model-matrix
        glm::vec3 pos = target_obj->get_position();
        light_data[ idx     ] = static_cast<int>(light_type());
        light_data[ idx + 1 ] = pos.x;
        light_data[ idx + 2 ] = pos.y;
        light_data[ idx + 3 ] = pos.z;
        auto color_info = get_light_color();
        light_data[ idx + 4 ] = color_info.first.r;
        light_data[ idx + 5 ] = color_info.first.g;
        light_data[ idx + 6 ] = color_info.first.b;
        light_data[ idx + 7 ] = color_info.first.a;
        light_data[ idx + 8 ] = color_info.second;
        light_data[ idx + 9 ] = light_direction.x;
        light_data[ idx + 10 ] = light_direction.y;
        light_data[ idx + 11 ] = light_direction.z;
        idx += 12;
    }
    light_data[ idx++ ] = cut_off;
    light_data[ idx ] = out_cutoff;
    return light_data;
}


flash_light::flash_light(opengl_play::camera_ptr camera,
                         glm::vec4 color,
                         GLfloat strength,
                         GLfloat cut_off_angle,
                         GLfloat out_cutoff_angle) :
    spot_light::spot_light(glm::vec3(0.0), //Do not matter
                           color,
                           strength,
                           glm::vec3(0.0), //Do not matter
                           cut_off_angle,
                           out_cutoff_angle),
    camera_ptr{ camera }
{
    LOG3("Creating a new flash_light");
}

const std::vector<GLfloat> &flash_light::get_light_data()
{
    /*
     * Update the camera position and direction on the
     * basis of the camera position
     */
    glm::vec3 new_position = camera_ptr->get_position(),
            new_direction = camera_ptr->get_camera_front();
    set_position(new_position);
    light_direction = new_direction;
    return spot_light::get_light_data();
}

}
