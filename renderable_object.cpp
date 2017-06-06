#include <renderable_object.hpp>
#include <algorithm>
#include <iostream>
#include <logger/logger.hpp>
#include <factory.hpp>

namespace renderer
{

constexpr std::size_t RENDR_BUF_CONTENT_SIZE{ 100000 };
constexpr std::size_t RENDR_BUF_DEFAULT_HEAD_POS{ 90000 };


Renderable::Renderable()
{
    view_configuration.configure( View_config::supported_configs::world_space_coord );
    LOG0("New renderable, ID: ", id);
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

Rendr_data_buffer::Rendr_data_buffer()
{
    LOG3("Creating the rendering data buffer, size: ",
         RENDR_BUF_CONTENT_SIZE,", default head position:",
         RENDR_BUF_DEFAULT_HEAD_POS);
    /*
     * This can be tuned to allow more elements.
     */
    try {
        rendr_content_buffer = std::make_unique< Rendr::raw_pointer[] >( RENDR_BUF_CONTENT_SIZE );
    } catch( std::exception& ex ) {
        PANIC("Failed to allocate the buffer for the renderable object.");
    }
    rendr_content = rendr_content_buffer.get();
    buffer_head = RENDR_BUF_DEFAULT_HEAD_POS + 1;
    buffer_tail = RENDR_BUF_DEFAULT_HEAD_POS;
}

void Rendr_data_buffer::add_new_rendr( Rendr::pointer &rendr )
{
    /*
     * object with view mode set to camera_space_coord
     * should be rendered last.
     */
    if( false == rendr->object->view_configuration.is_camera_space() ) {
        LOG0("Adding new rendr with ID:",
             rendr->id," at the HEAD of the buffer, IDX:",
             buffer_head);
        if( buffer_head == 0 ) {
            //TODO: Those panic can be removed.. in the future..
            //Just shift the buffer or whatever else..
            PANIC("No more buffer space at the head.");
        }
        rendr_content[ --buffer_head ] = rendr.get();
    } else {
        LOG0("Adding new rendr with ID:",
             rendr->id," at the TAIL of the buffer, IDX:",
             buffer_tail);
        if( buffer_tail == RENDR_BUF_CONTENT_SIZE - 1 ) {
            PANIC("No more buffer space at the tail!");
        }
        rendr_content[ ++buffer_tail ] = rendr.get();
    }
}

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
    frustum_raw_ptr = frustum.get();
}

types::id_type Core_renderer::add_renderable( Renderable::pointer object )
{
    if( nullptr == object ) {
        ERR("Invalid renderable provided");
        return -1;
    }
    LOG0("Adding new renderable, ID ",
         object->id, ", name: ",
         object->nice_name());
    Rendr::pointer new_rendr = factory< Rendr >::create();
    new_rendr->object = object.get();
    new_rendr->object->set_shader( shader.get() );
    renderables[ new_rendr->id ] = new_rendr;
    LOG0("Assigned ID: ", new_rendr->id );
    /*
     * object with view mode set to camera_space_coord
     * should be rendered last.
     */
    rendr_data.add_new_rendr( new_rendr );
    /*
    if( object->view_configuration.is_camera_space() ) {
        rendering_content.insert( rendering_content.begin() , new_rendr.get() );
    } else {
        rendering_content.push_back( new_rendr.get() );
    }*/
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
    int_fast32_t buffer_idx_cnt = 0;
    for( int_fast64_t idx = rendr_data.buffer_head;
         idx <= rendr_data.buffer_tail;
         ++idx )
    {
        Rendr::raw_pointer cur = rendr_data.rendr_content[ idx ];
        if( Rendering_state::states::rendering_enabled !=
            cur->object->rendering_state.current() ) {
            return false;
        }
        /*
         * We need all those variables only in the first rendering loop
         */
        const bool is_camera_space = cur->object->view_configuration.is_camera_space();
        if( false == is_camera_space &&
            frustum_raw_ptr->is_inside( cur->object->rendering_data.position ) < 0.0f ) {
            continue;
        }
        prepare_for_rendering( cur );
        /*
         * Save the index of the renderables that are going to be
         * rendered, not need to loop throught all the twice.
         */
        rendering_content_idx_buffer[ buffer_idx_cnt++ ] = idx;
        prepare_rendr_color( cur );
        cur->object->render( );
        cur->object->clean_after_render( );
        ++num_of_render_op;
    }
    /*
     * Second loop.
     */
    model_picking->prepare_to_update();
    for( int_fast32_t idx = 0 ; idx < buffer_idx_cnt; ++idx )
    {
        Rendr::raw_pointer cur = rendr_data.rendr_content[ rendering_content_idx_buffer[ idx ] ];
        prepare_for_rendering( cur );
        model_picking->update( cur->object );
        cur->object->clean_after_render( );
        ++num_of_render_op;
    }
    model_picking->complete_update();
    return num_of_render_op;
}

lighting::lighting_pointer Core_renderer::scene_lights()
{
    return game_lights;
}

Model_picking::pointer Core_renderer::picking()
{
    return model_picking;
}

void Core_renderer::clear()
{
    framebuffers->clear();
}

bool Core_renderer::prepare_for_rendering( Rendr::raw_pointer cur )
{
    const bool is_camera_space = cur->object->view_configuration.is_camera_space();

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

void Core_renderer::prepare_rendr_color( Rendr::raw_pointer cur ) const
{
    types::color color = cur->object->rendering_data.default_color;

    glUniform4f(config.color_loc,
                color.r,
                color.g,
                color.b,
                color.a);
}

void Core_renderer::switch_proper_perspective(
        const Renderable::raw_pointer obj
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
    framebuffers{ framebuffers }
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
    LOG0("Adding new object with color code: ", color_code,
         ", for the color: ", color_operations.denormalize_color( assigned_color ) );
    rendrid_to_color[ object->id ] = color_code;
    color_to_rendr[ color_code ] = object;
    return assigned_color;
}

std::size_t Model_picking::pick(
        const GLuint x,
        const GLuint y)
{
    LOG3("New 'simple' pick request: ",
         x,"/",y,". queue size: ",
         pick_requests.size());
    pick_requests.push_back( { x, y , pick_type::simple });
    return pick_requests.size();
}

std::size_t Model_picking::pick_toggle(
        const GLuint x,
        const GLuint y )
{
    LOG3("New 'toggle' pick request: ",
         x,"/",y,". queue size: ",
         pick_requests.size());
    pick_requests.push_back( { x, y , pick_type::toggle });
    return pick_requests.size();
}

void Model_picking::unpick()
{
    selected.removel_all();
}

std::vector< Renderable::pointer > Model_picking::get_selected()
{
    auto rendrs =  selected.get_selected();
    std::vector< Renderable::pointer > ret;
    for( auto&& sel : rendrs ) {
        ret.push_back( sel->object );
    }
    return ret;
}

std::vector<types::id_type> Model_picking::get_selected_ids()
{
    auto rendrs =  selected.get_selected();
    std::vector< types::id_type > ids;
    for( auto&& sel : rendrs ) {
        ids.push_back( sel->object->id );
    }
    return ids;
}

void Model_picking::update(
        const Renderable::raw_pointer object
        ) const
{
    const auto it = rendrid_to_color.find( object->id );
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

void Model_picking::complete_update()
{
    /*
     * Good opportunity to update whatever need
     * to be updated before cleaning up
     */
    if( pointed_model.update_required ) {
        pointed_model.pointed = model_at_position( pointed_model.x,
                                           pointed_model.y );
    }
    process_pick_requests();
    //Clean up
    game_shader->enable_texture_calculations();
    game_shader->enable_light_calculations();

    /*
     * return to default framebuffer
     */
    framebuffers->unbind();
}

void Model_picking::process_pick_requests()
{
    for( auto&& req : pick_requests ) {
        auto obj = model_at_position( req.x, req.y );
        if( obj == nullptr ) {
            continue;
        }
        if( req.type == pick_type::simple ) {
            LOG0("Picking model with ID: ", obj->id );
            selected.add( obj );
        } else if( req.type == pick_type::toggle ) {
            if( true == selected.is_selected( obj ) ) {
                LOG0("Model with ID:", obj->id,
                     " already selected, unselecting.");
                //Already selected, toggle selection
                selected.remove( obj );
            } else {
                LOG0("Picking model with ID: ", obj->id,
                     ". Position: ", obj->rendering_data.position);
                selected.add( obj );
            }
        }
    }
    pick_requests.clear();
}

Renderable::pointer Model_picking::model_at_position(
        const GLuint x,
        const GLuint y )
{
    /*
     * Read the color from the model picking
     * frame buffer
     */
    GLubyte pixels[3] = { 0,0,0 };
    glReadPixels( x, y,
                  1, 1,
                  GL_RGB,
                  GL_UNSIGNED_BYTE,
                  &pixels);
    /*
     * Attempt to find the model corresponding
     * to the color
     */
    const types::color color( pixels[0], pixels[1], pixels[2], 1.0f );
    const uint64_t color_code = color_operations.get_color_code( color );
    auto it = color_to_rendr.find( color_code );
    if( it != color_to_rendr.end() ) {
        return it->second;
    }
    return nullptr;
}

void Model_picking::set_pointed_model(
        const GLuint x,
        const GLuint y)
{
    pointed_model.x = x;
    pointed_model.y = y;
    pointed_model.update_required = true;
}

Renderable::pointer Model_picking::get_pointed_model() const
{
    return pointed_model.pointed;
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

void Selected_models::add( Renderable::pointer object )
{
    LOG0("Adding new selected model, ID: ",
         object->id,
         ", current size: ", selected.size() );
    if( find( object ) != selected.end() ) {
        LOG0("Object already selected, cannot select twice!");
        return;
    }
    auto new_model_info = factory< Selected_model_info >::create(
                object->rendering_data.default_color,
                object
                );
    selected.push_back( new_model_info );
    /*
     * Change the default color
     * to highlight the model
     */
    object->rendering_data.default_color.r *= 20.0f;
}

bool Selected_models::remove( Renderable::pointer object )
{
    auto it = find( object );
    if( it != selected.end() ) {
        LOG0("Removing selected model, ID:",
             object->id);
        object->rendering_data.default_color = (*it)->original_color;
        selected.erase( it );
        return true;
    }
    return false;
}

std::size_t Selected_models::removel_all()
{
    for( auto&& obj : selected ) {
        obj->object->rendering_data.default_color = obj->original_color;
    }
    const auto cnt = selected.size();
    selected.clear();
    return cnt;
}

std::size_t Selected_models::count() const
{
    return selected.size();
}

const Selected_model_info::container &Selected_models::get_selected()
{
    return selected;
}

bool Selected_models::is_selected(const Renderable::pointer &obj)
{
    return find( obj ) != selected.end();
}

Selected_model_info::container::iterator Selected_models::find(
        const Renderable::pointer &obj)
{
    for( auto it = selected.begin() ; it != selected.end() ; ++it ) {
        if( (*it)->object == obj ) {
            return it;
        }
    }
    return selected.end();
}

Renderable::pointer Core_renderer_proxy::pointed_model() const
{
    return core_renderer->picking()->get_pointed_model();
}

}
