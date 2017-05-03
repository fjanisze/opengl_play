#include <my_lines.hpp>

namespace opengl_play
{

my_simple_lines::my_simple_lines() :
    next_line_id{ 1 }
{
    LOG1("my_lines::my_lines(): Construction");
    shaders.load_vertex_shader(
                shaders.read_shader_body("../line_shader.vert"));
    shaders.load_fragment_shader(
                shaders.read_shader_body("../line_shader.frag"));

    if(!shaders.create_shader_program()){
        ERR("Unable to create the shader program!");
        throw std::runtime_error("Shader program creation failure!");
    }

    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
}

my_simple_lines::~my_simple_lines()
{
    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
}

int my_simple_lines::add_line(glm::vec3 from, glm::vec3 to,
                              glm::vec3 color)
{
    single_line new_line = {
        {from.x,from.y,from.z},
        {color.r,color.b,color.g},
        {to.x,to.y,to.z},
        {color.r,color.b,color.g}
    };
    lines.push_back(new_line);
    line_data_idx[ next_line_id ] = lines.size() - 1;
    update_buffers();
    LOG2("Line with ID: ",next_line_id," idx:",
         line_data_idx[ next_line_id ],
         " added. Amount of lines: ", lines.size());
    return next_line_id++;
}

void my_simple_lines::set_transformations(glm::mat4 v,
                                          glm::mat4 p)
{
    view = v;
    projection = p;
}

void my_simple_lines::update_buffers()
{
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 lines.size() * sizeof(single_line),
                 lines.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
                          6 * sizeof(GLfloat),
                          (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (GLvoid*)(3* sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
}

void my_simple_lines::modify_view(glm::mat4 new_view)
{
    view = new_view;
}

void my_simple_lines::prepare_for_render()
{
    shaders.use_shaders();
    GLint view_loc = glGetUniformLocation(shaders,"view");
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    GLint projection_loc = glGetUniformLocation(shaders,"projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model;
    GLint model_loc = glGetUniformLocation(shaders,"model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

}

void my_simple_lines::render()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES,0,lines.size() * 2);
    glBindVertexArray(0);
}

void my_simple_lines::clean_after_render()
{

}

std::string my_simple_lines::nice_name()
{
    return "static_line";
}

const single_line * const my_simple_lines::get_line_info(int id) const
{
    auto it = line_data_idx.find( id );
    if( it == line_data_idx.end() ) {
        return nullptr;
    }
    return (lines.data() + it->second);
}

bool my_simple_lines::modify_line_endpoints(int id,
                                            const glm::vec3 &origin,
                                            const glm::vec3 &end)
{
    auto it = line_data_idx.find( id );
    if( it == line_data_idx.end() ) {
        return false;
    }
    int idx = line_data_idx[ it->second ];
    lines[ idx ].from[0] = origin.x;
    lines[ idx ].from[1] = origin.y;
    lines[ idx ].from[2] = origin.z;
    lines[ idx ].to[0] = end.x;
    lines[ idx ].to[1] = end.y;
    lines[ idx ].to[2] = end.z;
    update_buffers();
    return true;
}

int my_simple_lines::get_invalid_id()
{
    return 0;
}

}
