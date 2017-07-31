#ifndef SHADERS_HPP
#define SHADERS_HPP

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include "logger/logger.hpp"
#include <memory>

namespace shaders {

class Shader
{
public:
    using pointer = std::shared_ptr< Shader >;
    using raw_poiner = Shader*;
    Shader();
    void load_vertex_shader( const std::string& body );
    void load_fragment_shader( const std::string& body );
    std::string read_shader_body( const std::string& filename );
    bool create_shader_program();
    void use_shaders();
    GLuint get_program() const;

    operator GLuint()
    {
        return shader_program;
    }

    /*
     * Load from the shader an uniform
     * or raise an error
     */
    GLint load_location( const std::string& loc_name );

    void enable_light_calculations();
    void disable_light_calculations();
    void enable_texture_calculations();
    void disable_texture_calculations();
private:
    GLuint vertex_shader,
           fragment_shader,
           shader_program;
    GLuint light_calc_uniform;
    GLuint tex_calc_uniform;
    GLchar log_buffer[512];
    void load_shader_generic( GLuint& shader_target,
                              const std::string& body,
                              GLenum shader_type );
};

}

#endif
