#include "shaders.hpp"

namespace shaders
{

void Shader::load_shader_generic(GLuint &shader_target,
                                           const std::string &body,
                                           GLenum shader_type)
{
    const char* body_ptr = body.c_str();
    LOG3("Compiling shader: ",
         shader_type);
    shader_target = glCreateShader(shader_type);
    glShaderSource(shader_target,
                   1,
                   &body_ptr,
                   nullptr);
    glCompileShader(shader_target);
    GLint success;
    glGetShaderiv(shader_target,
                  GL_COMPILE_STATUS,
                  &success);
    if(!success){
        ERR("Compilation failed!");
        glGetShaderInfoLog(shader_target,
                           512,
                           nullptr,
                           log_buffer);
        ERR("Detailed information: ",log_buffer);
    }
}

Shader::Shader()
{
    LOG3("Creating the shader compiler");

}

void Shader::load_vertex_shader(const std::string &body)
{
    load_shader_generic(vertex_shader,
                        body,
                        GL_VERTEX_SHADER);
}

void Shader::load_fragment_shader(const std::string &body)
{
    load_shader_generic(fragment_shader,
                        body,
                        GL_FRAGMENT_SHADER);
}

std::string Shader::read_shader_body(const std::string &filename)
{
    std::ifstream in_file(filename.c_str());
    std::string shader_body;
    if(!in_file){
        ERR("Unable to open the file ",
            filename);
    }
    else
    {
        std::size_t file_size = 0;
        in_file.seekg(0, std::ios_base::end);
        file_size = in_file.tellg();
        in_file.seekg(0,std::ios_base::beg);

        try{
            shader_body.resize(file_size);
            in_file.read(&shader_body[0],
                    file_size);
        }catch(std::exception& ex){
            ERR(ex.what());
            throw;
        }


    }

    return shader_body;
}

bool Shader::create_shader_program()
{
    LOG3("Creating the shader program");
    shader_program = glCreateProgram();
    //Attach our two shaders
    glAttachShader(shader_program,
                   vertex_shader);
    glAttachShader(shader_program,
                   fragment_shader);
    //Link the shaders
    glLinkProgram(shader_program);
    //Check for errors
    GLint success;
    glGetProgramiv(shader_program,
                   GL_LINK_STATUS,
                   &success);
    if(!success){
        ERR("Linking failed!");
        glGetShaderInfoLog(shader_program,
                           512,
                           nullptr,
                           log_buffer);
        ERR("Detailed information: ",log_buffer);
        return false;
    }

    light_calc_uniform = load_location("skip_light_calculations");
    tex_calc_uniform = load_location("skip_texture_calculations");
    return true;
}

void Shader::use_shaders()
{
    glUseProgram(shader_program);
}

GLuint Shader::get_program( ) const
{
    return shader_program;
}

GLint Shader::load_location(const std::string &loc_name)
{
    LOG1("Loading location: ", loc_name );
    GLint loc = glGetUniformLocation( shader_program,
                               loc_name.c_str() );
    if( loc < 0 ) {
        PANIC("Unable to load the shader uniform: " + loc_name );
    }
    return loc;
}

void Shader::enable_light_calculations()
{
    glUniform1i(light_calc_uniform, 0);
}

void Shader::disable_light_calculations()
{
    glUniform1i(light_calc_uniform, 1);
}

void Shader::enable_texture_calculations()
{
    glUniform1i(tex_calc_uniform, 0);
}

void Shader::disable_texture_calculations()
{
    glUniform1i(tex_calc_uniform, 1);
}


}
