#include "shaders.hpp"

namespace shaders
{

void my_small_shaders::load_shader_generic(GLuint &shader_target,
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

my_small_shaders::my_small_shaders()
{
    LOG3("Creating the shader compiler");

}

void my_small_shaders::load_vertex_shader(const std::string &body)
{
    load_shader_generic(vertex_shader,
                        body,
                        GL_VERTEX_SHADER);
}

void my_small_shaders::load_fragment_shader(const std::string &body)
{
    load_shader_generic(fragment_shader,
                        body,
						GL_FRAGMENT_SHADER);
}

std::string my_small_shaders::read_shader_body(const std::string &filename)
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

bool my_small_shaders::create_shader_program()
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
    return true;
}

void my_small_shaders::use_shaders()
{
    glUseProgram(shader_program);
}

GLuint my_small_shaders::get_program()
{
    return shader_program;
}


}
