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

inline
renderable_state Renderable::get_rendering_state() const
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

    view_loc = shader->load_location("view");
    projection_loc = shader->load_location("projection");

    cur_perspective = perspective_type::projection;
    glUniformMatrix4fv(projection_loc, 1,
                       GL_FALSE, glm::value_ptr(projection));

    model_loc = shader->load_location("model");
    color_loc = shader->load_location("object_color");

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
    long num_of_render_op{ 0 };
    game_lights->calculate_lighting( shader );
    glm::mat4 view = camera->get_view();

    bool def_view_matrix_loaded{ true };
    glUniformMatrix4fv(view_loc, 1,
                       GL_FALSE, glm::value_ptr(view));

    /*
     * The rendering loop is performed twice,
     * once for the rendering to the default framebuffer
     * the second time in order to update the mouse picking
     * data
     */
    for( int rendr_loop{1} ; rendr_loop <= 2 ; ++rendr_loop )
    {
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

            cur->object->prepare_for_render( shader );
            /*
             * First loop: Default framebuffer rendering,
             * Second loop: Mouse picking
             */
            if( rendr_loop == 1 ) {
                glm::vec4 color = cur->object->default_color;
                /*
                 * If this Renderable is currencly picked
                 * increase a little bit it's default color
                 * to increase it's visibility
                 */
                if( model_picking->get_selected() ==
                    cur->object->id ) {
                    color *= 1.4f;
                    color.a = 1.0f;
                }
                glUniform4f(color_loc,
                            color.r,
                            color.g,
                            color.b,
                            color.a);

                cur->object->render( shader );
            } else {
                model_picking->update( cur->object );
            }
            ++num_of_render_op;
            cur->object->clean_after_render( shader );
        }
        /*
         * After the first loop we're going to
         * repeat the process to update the mouse
         * picking information
         */
        model_picking->prepare_to_update();
    }
    /*
     * Model_picking is the last rendering
     * stuff, we nee to tell him to clean up
     * after he's done.
     */
    model_picking->cleanup_after_update();
    return num_of_render_op;
}

lighting::lighting_pointer Core_renderer::scene_lights()
{
    return game_lights;
}

Renderable::pointer Core_renderer::select_model(const GLuint x,
                                                   const GLuint y)
{
    return model_picking->pick( x, y );
}

uint64_t Core_renderer::get_selected_model() const
{
    return model_picking->get_selected();
}

void Core_renderer::clear()
{
    framebuffers->clear();
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
    framebuffers{ framebuffers },
    cur_selected_renderable{ 0 }
{
    LOG3("Creating a new Model_picking object");
    picking_buffer_id = framebuffers->create_buffer();
    shader_color_loc = game_shader->load_location("object_color");
}

types::color Model_picking::add_model(
        Renderable::pointer object )
{
    types::color assigned_color = color_operations.get_color();
    uint64_t color_code = color_operations.get_color_code(
                color_operations.denormalize_color( assigned_color )
                );
    auto it = color_to_rendr.find( color_code );
    if( it != color_to_rendr.end() ) {
        PANIC("Not able to generate unique color codes.");
    }
    LOG3("Adding new object with color code: ", color_code,
         ", for the color: ", color_operations.denormalize_color( assigned_color ) );
    rendrid_to_color[ object->id ] = color_code;
    color_to_rendr[ color_code ] = object;
    return assigned_color;
}

Renderable::pointer Model_picking::pick(
        const GLuint x,
        const GLuint y)
{
    /*
     * Read the color from the model picking
     * frame buffer
     */
    framebuffers->bind( picking_buffer_id );
    GLubyte pixels[3] = { 0,0,0 };
    glReadPixels( x, y,
                  1, 1,
                  GL_RGB,
                  GL_UNSIGNED_BYTE,
                  &pixels);
    framebuffers->unbind();
    /*
     * Attempt to find the model corresponding
     * to the color
     */
    types::color color( pixels[0], pixels[1], pixels[2], 1.0f );
    uint64_t color_code = color_operations.get_color_code( color );
    auto it = color_to_rendr.find( color_code );
    //LOG3("Color: ", color, ". code: ", color_code, ". Found: ", ( it != color_to_rendr.end()) );
    if( color_to_rendr.end() != it ) {
        cur_selected_renderable = it->second->id;
        return it->second;
    }
    unpick();
    return nullptr;
}

void Model_picking::unpick()
{
    cur_selected_renderable = 0;
}

uint64_t Model_picking::get_selected() const
{
    return cur_selected_renderable;
}

void Model_picking::update(
        const Renderable::pointer &object
        )
{
    auto it = rendrid_to_color.find( object->id );
    if( rendrid_to_color.end() != it ) {
        //The object exist in our 'database'
        auto color = color_operations.get_color_rgba( it->second );
        color = color_operations.normalize_color( color );

        glUniform4f(shader_color_loc,
                    color.r,
                    color.g,
                    color.b,
                    color.a);

        object->render( game_shader );
    }
}

void Model_picking::prepare_to_update()
{
    /*
     * Render the model in our model picking
     * framebuffer
     */
    framebuffers->bind( picking_buffer_id );

    game_shader->disable_light_calculations();
    game_shader->disable_texture_calculations();
}

void Model_picking::cleanup_after_update()
{
    game_shader->enable_texture_calculations();
    game_shader->enable_light_calculations();

    /*
     * return to default framebuffer
     */
    framebuffers->unbind();
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

types::color Color_creator::denormalize_color( types::color color ) const
{
    for( int i {0}; i < 3; ++i ) {
        color[ i ] = std::floor( color[ i ] * 255.0f + 0.5f );
    }
    return color;
}

types::color Color_creator::normalize_color( types::color color ) const
{
    color /= 255.0f;
    color.a = 1.0f;
    return color;
}

uint64_t Color_creator::get_color_code(const types::color &color)
{
    return (uint64_t)color.r << 16 |
           (uint64_t)color.g << 8 |
           (uint64_t)color.b;
}

types::color Color_creator::get_color_rgba(const uint64_t color_code) const
{
    types::color color;
    color.r = ( color_code >> 16 ) & 0xFF;
    color.g = ( color_code >> 8 ) & 0xFF;
    color.b = ( color_code ) & 0xFF;
    color.a = 1.0f;
    return color;
}


}
