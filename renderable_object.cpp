#include <renderable_object.hpp>
#include <algorithm>
#include <iostream>
#include <logger/logger.hpp>
#include <factory.hpp>

namespace renderer
{

Renderable::Renderable()
{
    view_configuration.configure( View_config::supported_configs::world_space_coord );
    LOG1("New renderable, ID: ", rendering_data.id);
}

void Renderable::set_shader(shaders::Shader::raw_poiner shader)
{
    rendering_data.shader = shader;
}

std::string Renderable::nice_name()
{
    return "(nice name not provided)";
}

//////////////////////////////////////
/// core_renderer
/////////////////////////////////////

Core_renderer::Core_renderer(const types::win_size &window,
                             const glm::mat4 &proj,
                             const glm::mat4& def_ortho,
                             const scene::Camera::pointer cam ) :
    config( window ),
    camera{ cam }
{
    LOG3("Creating the core renderer!");
    config.projection = proj;
    config.ortho = def_ortho;
    shader = factory< shaders::Shader >::create();

    shader->load_fragment_shader(shader->read_shader_body(
                                     "../model_shader.frag"));
    shader->load_vertex_shader(shader->read_shader_body(
                               "../model_shader.vert"));
    if( !shader->create_shader_program() ) {
        ERR("Unable to create the model shader!");
        throw std::runtime_error("Shader creation failure");
    }

    shader->use_shaders();

    config.view_loc = shader->load_location("view");
    config.projection_loc = shader->load_location("projection");

    config.cur_perspective = perspective_type::projection;
    glUniformMatrix4fv(config.projection_loc, 1,
                       GL_FALSE, glm::value_ptr(config.projection));

    config.model_loc = shader->load_location("model");
    config.color_loc = shader->load_location("object_color");

    framebuffers = factory< buffers::Framebuffers >::create(
                window );
    game_lights = std::make_shared< lighting::Core_lighting >();
    model_picking = factory< Model_picking >::create( shader, framebuffers );

    frustum = factory< scene::Frustum >::create( camera,
                            45.0f,
                            (GLfloat)window.width / (GLfloat)window.height,
                            1.0f,
                            100.0f);
}

types::id_type Core_renderer::add_renderable( Renderable::pointer object )
{
    if( nullptr == object ) {
        ERR("Invalid renderable provided");
        return -1;
    }
    LOG3("Adding new renderable, ID ",
         object->rendering_data.id, ", name: ",
         object->nice_name());
    Rendr::pointer new_rendr = factory< Rendr >::create();
    new_rendr->object = object;
    new_rendr->object->set_shader( shader.get() );
    renderables[ new_rendr->id ] = new_rendr;
    /*
     * object with view mode set to camera_space_coord
     * should be rendered last.
     */
    if( object->view_configuration.is_camera_space() ) {
        rendering_content.insert( rendering_content.begin() , new_rendr.get() );
    } else {
        rendering_content.push_back( new_rendr.get() );
    }
    LOG1("Assigned ID: ", new_rendr->id );
    model_picking->add_model( object );
    return new_rendr->id;
}

long Core_renderer::render()
{
    long num_of_render_op{ 0 };
    game_lights->calculate_lighting( shader );
    frustum->update();
    config.view_matrix = camera->get_view();

    config.is_def_view_matrix_loaded = true;
    glUniformMatrix4fv(config.view_loc, 1,
                       GL_FALSE, glm::value_ptr(config.view_matrix));

    /*
     * The rendering loop is performed twice,
     * once for the rendering to the default framebuffer
     * the second time in order to update the mouse picking
     * data
     */
    for( int_fast64_t idx = rendering_content.size() - 1;
         idx >= 0 ;
         --idx )
    {
        Rendr::raw_pointer cur = rendering_content[ idx ];
        if( Rendering_state::states::rendering_enabled !=
            cur->object->rendering_state.current() ) {
            return false;
        }
        if( false == prepare_for_rendering( cur ) ) {
            /*
             * To avoid a second check on whether the object
             * is inside the frustum (during the second loop)
             * set the state of the Renderable to not_visible.
             */
            cur->object->rendering_state.set_not_visible();
            continue;
        }
        prepare_rendr_color( cur );
        cur->object->render( );
        cur->object->clean_after_render( );
        ++num_of_render_op;
    }
    /*
     * Second loop.
     */
    model_picking->prepare_to_update();
    for( int_fast64_t idx = rendering_content.size() - 1;
         idx > 0 ;
         --idx )
    {
        Rendr::raw_pointer cur = rendering_content[ idx ];
        if( renderer::Rendering_state::states::not_visible ==
            cur->object->rendering_state.current() ) {
            /*
             * Set the state back to enabled, in
             * the next rendering iteration we will know
             * if the object is still not visible.
             */
            cur->object->rendering_state.set_enable();
            continue;
        }
        if( Rendering_state::states::rendering_enabled !=
            cur->object->rendering_state.current() ) {
            return false;
        }
        if( false == prepare_for_rendering( cur ) ) {
            continue;
        }
        model_picking->update( cur->object );
        cur->object->clean_after_render( );
        ++num_of_render_op;
    }
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

bool Core_renderer::prepare_for_rendering( Rendr::raw_pointer cur )
{
    const glm::mat4& model = cur->object->rendering_data.model_matrix;
    const glm::vec3 pos(glm::vec3(model[3].x,model[3].y,model[3].z));
    const bool is_camera_space = cur->object->view_configuration.is_camera_space();

    /*
     * The value of is_inside is used to tune the amount of objects
     * rendered. The problem is that the rendered is stupid, to consider
     * an object out of the frustum is sufficient that the border point
     * if out of it! To avoid 'holes' of non rendered object on the border
     * of the frustum the renderer render a little behing the frustum planes.
     */
    if( false == is_camera_space && frustum->is_inside( pos ) < 0.0f ) {
        return false;
    }

    switch_proper_perspective( cur->object );

    glUniformMatrix4fv(config.model_loc, 1, GL_FALSE,
                       glm::value_ptr( cur->object->rendering_data.model_matrix ) );
    if( is_camera_space ) {
        glUniformMatrix4fv(config.view_loc, 1,
                           GL_FALSE, glm::value_ptr(
                               cur->object->rendering_data.model_matrix
                               ));
        config.is_def_view_matrix_loaded = false;
    } else if( false == config.is_def_view_matrix_loaded ){
        glUniformMatrix4fv(config.view_loc, 1,
                           GL_FALSE, glm::value_ptr(config.view_matrix));
        config.is_def_view_matrix_loaded = true;
    }

    cur->object->prepare_for_render( );
    return true;
}

void Core_renderer::prepare_rendr_color( Rendr::raw_pointer cur )
{
    types::color color = cur->object->rendering_data.default_color;
    /*
     * If this Renderable is currencly picked
     * increase a little bit it's default color
     * to increase it's visibility
     */
    if( model_picking->get_selected() ==
        cur->object->rendering_data.id ) {
        color *= 1.4f;
        color.a = 1.0f;
    }

    glUniform4f(config.color_loc,
                color.r,
                color.g,
                color.b,
                color.a);
}

void Core_renderer::switch_proper_perspective(
        const Renderable::pointer &obj
        )
{
    if( obj->view_configuration.is_camera_space() &&
        perspective_type::projection == config.cur_perspective ) {
        /*
         * Need to switch from projection to ortho
         */
        glUniformMatrix4fv(config.projection_loc, 1,
                           GL_FALSE, glm::value_ptr(config.ortho));
        config.cur_perspective = perspective_type::ortho;
    } else if ( obj->view_configuration.is_world_space() &&
               perspective_type::ortho == config.cur_perspective) {
        /*
         * Need to switch from ortho to projection
         */
        glUniformMatrix4fv(config.projection_loc, 1,
                           GL_FALSE, glm::value_ptr(config.projection));
        config.cur_perspective = perspective_type::projection;
    }
}

//////////////////////////////////////
/// Model_picking
/////////////////////////////////////

Model_picking::Model_picking(shaders::Shader::pointer shader,
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
    LOG1("Adding new object with color code: ", color_code,
         ", for the color: ", color_operations.denormalize_color( assigned_color ) );
    rendrid_to_color[ object->rendering_data.id ] = color_code;
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
        cur_selected_renderable = it->second->rendering_data.id;
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
        ) const
{
    const auto it = rendrid_to_color.find( object->rendering_data.id );
    if( rendrid_to_color.end() != it ) {
        //The object exist in our 'database'
        auto color = color_operations.get_color_rgba( it->second );
        color = color_operations.normalize_color( color );

        glUniform4f(shader_color_loc,
                    color.r,
                    color.g,
                    color.b,
                    color.a);

        object->render( );
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
    return types::color( ( color_code >> 16 ) & 0xFF,
                         ( color_code >> 8 ) & 0xFF,
                         ( color_code ) & 0xFF,
                         1.0f );
}


}
