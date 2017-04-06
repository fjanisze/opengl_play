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

namespace shaders
{

class my_small_shaders;
using shader_ptr = std::shared_ptr< my_small_shaders >;

class my_small_shaders
{
public:
    my_small_shaders();
    void load_vertex_shader(const std::string& body);
    void load_fragment_shader(const std::string& body);
    std::string read_shader_body(const std::string& filename);
    bool create_shader_program();
    void use_shaders();
    GLuint get_program();
    static shader_ptr create() {
        return std::make_shared< my_small_shaders >();
    }

    /*
     * Configure the fragment shader to skip
     * all the calculations and to render
     * everything in one color
     */
    void force_single_color(const glm::vec3& color = glm::vec3(0.0));
    operator GLuint(){
        return shader_program;
    }
private:
    GLuint vertex_shader,
            fragment_shader,
            shader_program;
    GLchar log_buffer[512];
    void load_shader_generic(GLuint& shader_target,
                             const std::string& body,
                             GLenum shader_type);
};

}

#endif
