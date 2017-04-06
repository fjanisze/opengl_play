#include <renderable_object.hpp>
#include <algorithm>
#include <iostream>
#include <logger/logger.hpp>

namespace renderable
{

std::string renderable_object::renderable_nice_name()
{
    return "(nice name not provided)";
}

//////////////////////////////////////
/// core_renderer
/////////////////////////////////////

core_renderer::core_renderer(const glm::mat4 &proj,
                             const opengl_play::camera_ptr cam ) :
    next_rendr_id{ 1 },
    projection{ proj },
    camera{ cam }
{
    LOG3("Creating the core renderer!");
    shader = shaders::my_small_shaders::create();

    shader->load_fragment_shader(shader->read_shader_body(
                                     "../model_shader.frag"));
    shader->load_vertex_shader(shader->read_shader_body(
                               "../model_shader.vert"));
    if( !shader->create_shader_program() ) {
        ERR("Unable to create the model shader!");
        throw std::runtime_error("Shader creation failure");
    }

    shader->use_shaders();

    view_loc = glGetUniformLocation(*shader,"view");
    projection_loc = glGetUniformLocation(*shader,"projection");
    model_loc = glGetUniformLocation(*shader,"model");
    color_loc = glGetUniformLocation(*shader,"object_color");
}

renderable_id core_renderer::add_renderable( renderable_pointer object )
{
    if( nullptr == object ) {
        ERR("Invalid renderable provided");
        return -1;
    }
    LOG1("Adding new renderable: ",
         object->renderable_nice_name());
    rendr new_rendr;
    new_rendr.id = next_rendr_id;
    new_rendr.object = object;
    renderables[ next_rendr_id ] = new_rendr;
    ++next_rendr_id;
    LOG1("Assigned ID: ", new_rendr.id );
    return new_rendr.id;
}

long core_renderer::render()
{
    glm::mat4 view = camera->get_view();

    glUniformMatrix4fv(view_loc, 1,
                       GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projection_loc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    for( auto&& rendr : renderables )
    {
        glm::vec3 color = rendr.second.object->default_color;
        glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                           glm::value_ptr( rendr.second.object->model_matrix ));
        glUniform3f(color_loc,
                    color.r,
                    color.b,
                    color.g);
        rendr.second.object->prepare_for_render();
        rendr.second.object->render( shader );
        rendr.second.object->clean_after_render();
    }
}



}
