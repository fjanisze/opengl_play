#include <renderable_object.hpp>
#include <algorithm>
#include <iostream>
#include <logger/logger.hpp>

namespace renderable
{

renderable_object::renderable_object() :
    state{ renderable_state::rendering_disabled }
{
    model_matrix = glm::mat4();
    set_view_method( view_method::world_space_coord );
}

void renderable_object::set_rendering_state(const renderable_state new_state)
{
    state = new_state;
}

renderable_state renderable_object::get_rendering_state()
{
    return state;
}

void renderable_object::set_view_method(const view_method new_method)
{
    type_of_view = new_method;
    if( view_method::camera_space_coord == new_method ) {
        model_matrix = glm::mat4();
    }
}

std::string renderable_object::renderable_nice_name()
{
    return "(nice name not provided)";
}

//////////////////////////////////////
/// core_renderer
/////////////////////////////////////

core_renderer::core_renderer(const glm::mat4 &proj,
                             const glm::mat4& def_ortho,
                             const opengl_play::camera_ptr cam ) :
    next_rendr_id{ 1 },
    projection{ proj },
    ortho{ def_ortho },
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

    view_loc = load_location("view");
    projection_loc = load_location("projection");

    cur_perspective = perspective_type::projection;
    glUniformMatrix4fv(projection_loc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    model_loc = load_location("model");
    color_loc = load_location("object_color");

    game_lights = std::make_shared< lighting::Core_lighting >();
}

renderable_id core_renderer::add_renderable( renderable_pointer object )
{
    if( nullptr == object ) {
        ERR("Invalid renderable provided");
        return -1;
    }
    LOG1("Adding new renderable: ",
         object->renderable_nice_name());
    rendr_ptr new_rendr = std::make_shared<rendr>();
    new_rendr->id = next_rendr_id;
    new_rendr->object = object;
    renderables[ next_rendr_id ] = new_rendr;
    /*
     * object with view mode set to camera_space_coord
     * should be rendered last (tail of the rendering list)
     */
    if( nullptr == rendering_head ) {
        rendering_head = new_rendr;
        rendering_tail = new_rendr;
    } else {
        if( renderable::view_method::camera_space_coord == object->get_view_method() ) {
            rendering_tail->next = new_rendr;
            rendering_tail = new_rendr;
        } else {
            new_rendr->next = rendering_head;
            rendering_head = new_rendr;
        }
    }
    ++next_rendr_id;
    LOG1("Assigned ID: ", new_rendr->id );
    return new_rendr->id;
}

long core_renderer::render()
{
    game_lights->calculate_lighting( shader );
    glm::mat4 view = camera->get_view();

    bool def_view_matrix_loaded{ true };
    glUniformMatrix4fv(view_loc, 1,
                       GL_FALSE, glm::value_ptr(view));

    for( rendr_ptr cur = rendering_head ;
         cur != nullptr ;
         cur = cur->next )
    {
        if( cur->object->get_rendering_state() == renderable_state::rendering_disabled ) {
            continue;
        }

        switch_proper_perspective( cur->object );

        glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                           glm::value_ptr( cur->object->model_matrix ) );
        if( view_method::camera_space_coord == cur->object->get_view_method() ) {
            glUniformMatrix4fv(view_loc, 1,
                               GL_FALSE, glm::value_ptr(
                                   cur->object->model_matrix
                                   ));
            def_view_matrix_loaded = false;
        } else if( false == def_view_matrix_loaded ){
            glUniformMatrix4fv(view_loc, 1,
                               GL_FALSE, glm::value_ptr(view));
            def_view_matrix_loaded = true;
        }

        glm::vec4 color = cur->object->default_color;
        glUniform4f(color_loc,
                    color.r,
                    color.g,
                    color.b,
                    color.a);

        cur->object->prepare_for_render( shader );
        cur->object->render( shader );
        cur->object->clean_after_render( shader );
    }
}

lighting::lighting_pointer core_renderer::lights()
{
    return game_lights;
}

GLint core_renderer::load_location(const std::string &loc_name)
{
    LOG2("Loading location: ", loc_name );
    GLint loc = glGetUniformLocation( *shader,
                               loc_name.c_str() );
    if( loc < 0 ) {
        throw std::runtime_error(
                ("Unable to load the shader uniform: " + loc_name).c_str());
    }
    return loc;
}

void core_renderer::switch_proper_perspective(const renderable_pointer &obj)
{
    if( renderable::view_method::camera_space_coord ==
        obj->get_view_method() &&
        perspective_type::projection == cur_perspective ) {
        /*
         * Need to switch from projection to ortho
         */
        glUniformMatrix4fv(projection_loc, 1,
                           GL_FALSE, glm::value_ptr(ortho));
        cur_perspective = perspective_type::ortho;
    } else if (renderable::view_method::world_space_coord ==
               obj->get_view_method() &&
               perspective_type::ortho == cur_perspective) {
        /*
         * Need to switch from ortho to projection
         */
        glUniformMatrix4fv(projection_loc, 1,
                           GL_FALSE, glm::value_ptr(projection));
        cur_perspective = perspective_type::projection;
    }
}

}
