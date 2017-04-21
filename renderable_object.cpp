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
       // model_matrix = glm::mat4();
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

    view_loc = load_location("view");
    projection_loc = load_location("projection");


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
    rendr new_rendr;
    new_rendr.id = next_rendr_id;
    new_rendr.object = object;
    renderables[ next_rendr_id ] = new_rendr;
    /*
     * object with view mode set to camera_space_coord
     * should be rendered last (
     * */
    ++next_rendr_id;
    LOG1("Assigned ID: ", new_rendr.id );
    return new_rendr.id;
}

long core_renderer::render()
{
    game_lights->calculate_lighting( shader );
    glm::mat4 view = camera->get_view();
    glm::mat4 identity_matrix = glm::mat4();

    bool def_view_matrix_loaded{ true };
    glUniformMatrix4fv(view_loc, 1,
                       GL_FALSE, glm::value_ptr(view));

    for( auto&& rendr : renderables )
    {
        if( rendr.second.object->get_rendering_state() == renderable_state::rendering_disabled ) {
            continue;
        }

        glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                           glm::value_ptr( rendr.second.object->model_matrix ) );
        if( view_method::camera_space_coord == rendr.second.object->get_view_method() ) {
            glUniformMatrix4fv(view_loc, 1,
                               GL_FALSE, glm::value_ptr(
                                   rendr.second.object->model_matrix
                                   ));
            def_view_matrix_loaded = false;
        } else if( false == def_view_matrix_loaded ){
            glUniformMatrix4fv(view_loc, 1,
                               GL_FALSE, glm::value_ptr(view));
            def_view_matrix_loaded = true;
        }

        glm::vec3 color = rendr.second.object->default_color;
        glUniform3f(color_loc,
                    color.r,
                    color.g,
                    color.b);

        rendr.second.object->prepare_for_render();
        rendr.second.object->render( shader );
        rendr.second.object->clean_after_render();
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



}
