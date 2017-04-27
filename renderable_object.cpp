#include <renderable_object.hpp>
#include <algorithm>
#include <iostream>
#include <logger/logger.hpp>
#include <factory.hpp>

namespace renderer
{

Renderable::Renderable() :
    state{ renderable_state::rendering_disabled },
    id{ ids< Renderable >::create() }
{
    model_matrix = glm::mat4();
    set_view_method( view_method::world_space_coord );
    LOG1("New renderable, ID: ", id);
}

void Renderable::set_rendering_state(const renderable_state new_state)
{
    state = new_state;
}

renderable_state Renderable::get_rendering_state()
{
    return state;
}

void Renderable::set_view_method(const view_method new_method)
{
    type_of_view = new_method;
    if( view_method::camera_space_coord == new_method ) {
        model_matrix = glm::mat4();
    }
}

std::string Renderable::renderable_nice_name()
{
    return "(nice name not provided)";
}

//////////////////////////////////////
/// core_renderer
/////////////////////////////////////

Core_renderer::Core_renderer(const types::win_size &window,
                             const glm::mat4 &proj,
                             const glm::mat4& def_ortho,
                             const opengl_play::camera_ptr cam ) :
    next_rendr_id{ 1 },
    projection{ proj },
    ortho{ def_ortho },
    camera{ cam },
    viewport_size{ window }
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

    framebuffers = factory< buffers::Framebuffers >::create(
                window );
    game_lights = std::make_shared< lighting::Core_lighting >();
    model_picking = factory< Model_picking >::create( shader, framebuffers );
}

renderable_id Core_renderer::add_renderable( Renderable::pointer object )
{
    if( nullptr == object ) {
        ERR("Invalid renderable provided");
        return -1;
    }
    LOG1("Adding new renderable: ",
         object->renderable_nice_name());
    rendr_ptr new_rendr = std::make_shared<Rendr>();
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
        if( renderer::view_method::camera_space_coord == object->get_view_method() ) {
            rendering_tail->next = new_rendr;
            rendering_tail = new_rendr;
        } else {
            new_rendr->next = rendering_head;
            rendering_head = new_rendr;
        }
    }
    ++next_rendr_id;
    LOG1("Assigned ID: ", new_rendr->id );
    model_picking->add_model( object );
    return new_rendr->id;
}

long Core_renderer::render()
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

lighting::lighting_pointer Core_renderer::scene_lights()
{
    return game_lights;
}

GLint Core_renderer::load_location(const std::string &loc_name)
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

void Core_renderer::switch_proper_perspective(
        const Renderable::pointer &obj
        )
{
    if( renderer::view_method::camera_space_coord ==
        obj->get_view_method() &&
        perspective_type::projection == cur_perspective ) {
        /*
         * Need to switch from projection to ortho
         */
        glUniformMatrix4fv(projection_loc, 1,
                           GL_FALSE, glm::value_ptr(ortho));
        cur_perspective = perspective_type::ortho;
    } else if (renderer::view_method::world_space_coord ==
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

//////////////////////////////////////
/// Model_picking
/////////////////////////////////////

Model_picking::Model_picking(shaders::shader_ptr shader,
        buffers::Framebuffers::pointer framebuffers) :
    game_shader{ shader },
    framebuffers{ framebuffers }
{
    LOG3("Creating a new Model_picking object");
    picking_buffer_id = framebuffers->create_buffer();
}

types::color Model_picking::add_model(
        Renderable::pointer object )
{
    auto assigned_color = colors.get_color();
    auto color_code = get_color_code( assigned_color );
    LOG1("Adding new object with color code: ", color_code,
         ", for the color: ", assigned_color );
    rendrid_to_color[ object->id ] = color_code;
    color_to_rendr[ color_code ] = object;
    return assigned_color;
}

Renderable::pointer Model_picking::pick(
        const GLuint x,
        const GLuint y)
{

}

void Model_picking::unpick()
{

}

void Model_picking::update(
        const Renderable::pointer &object
        )
{
    auto it = rendrid_to_color.find( object->id );
    if( rendrid_to_color.end() != it ) {
        //The object exist in our 'database'

    }
}

uint64_t Model_picking::get_color_code(const types::color &color)
{
    auto den_color = colors.denormalize_color( color );
    return (uint64_t)den_color.r << 16 |
           (uint64_t)den_color.g << 8 |
           (uint64_t)den_color.b;
}

//////////////////////////////////////
/// Color_creator
/////////////////////////////////////

Color_creator::Color_creator( const GLfloat step ) :
    color_step{ step },
    next_color{ types::color( 1.0f ) },
    num_of_colors{ 0 }
{
    if( step <= 0.0f ) {
        PANIC("Negative step not allowed!");
    }
    max_num_of_colors = std::pow( glm::floor( 1.0f / step) , 3 );
    LOG3("Number of supported colors: ", max_num_of_colors);
}

types::color Color_creator::get_color()
{
    if( num_of_colors >= max_num_of_colors ) {
        PANIC("No more available colors, limit:",
              max_num_of_colors);
    }
    auto color = next_color;
    auto apply_step = [this]( GLfloat& component ) {
        component -= color_step;
        if( component < 0 ) {
            component = 1.0f;
        }
        return component;
    };

    /*
     * Calculate the next color, note that
     * if we complete all the available colors
     * give the color_step, then the generation
     * restart from the beginning (white color)
     */
    for( int i{ 2 } ; i >= 0 ; --i ) {
        if( apply_step( next_color[i] ) != 1.0f ) {
            break;
        }
    }
    ++num_of_colors;
    return color;
}

types::color Color_creator::denormalize_color( types::color color )
{
    for( int i {0}; i < 3; ++i ) {
        color[ i ] = std::floor( color[ i ] * 255.0f + 0.5f );
    }
    return color;
}

}
