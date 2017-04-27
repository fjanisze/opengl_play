#include <terrains.hpp>
#include <logger/logger.hpp>

namespace terrains
{

terrains::terrains(renderer::core_renderer_ptr rendr ) :
    renderer{ rendr }
{
    LOG3("Creating terrains::terrains");
    unselect_highlighted_lot();
}

long terrains::load_terrain(const std::string &model_filename,
                            const glm::vec4 &color,
                            long terrain_id)
{
    LOG3("Loading terrain model: ",
         model_filename,
         ". Provided ID: ",
         terrain_id);
    models::model_loader_ptr new_model = models::model_loader::load(
                model_filename);
    if( terrain_id < 0 ) {
        terrain_id = generate_unique_id();
    }
    if( terrain_id > 0 ) {
        terrain_container[ terrain_id ].low_res_model = new_model;
        terrain_container[ terrain_id ].high_res_model = new_model;
        terrain_container[ terrain_id ].default_color = color;
        LOG3("New terrain loaded, id: ", terrain_id,
             ". Amount of terrains: ", terrain_container.size() );
    } else {
        ERR("The provided terrain ID is already in use!");
        terrain_id = -1;
    }

    return terrain_id;
}

long terrains::load_highres_terrain(const std::string &model_filename,
                                    long terrain_id)
{
    LOG3("Loading highres terrain model: ",
         model_filename,
         ". Provided ID: ",
         terrain_id);
    auto it = terrain_container.find( terrain_id );
    if( it == terrain_container.end() ) {
        ERR("Not able to find the terrain with ID: ",terrain_id,
            ". Not possible to load the highres model..");
        return -1;
    }
    models::model_loader_ptr new_model = models::model_loader::load(
                model_filename);
    it->second.high_res_model = new_model;
    return terrain_id;
}

bool terrains::load_terrain_map(const terrain_map_t &map,
                                GLfloat lot_size,
                                glm::vec2 central_lot)
{
    if( lot_size <= 0 ){
        ERR("Lot size couldn't be zero!");
        return false;
    }
    if( map.empty() || map[0].empty() ) {
        ERR("The provided terrain map is empty!");
        return false;
    }
    this->lot_size = lot_size;
    origins_lot = central_lot;
    /*
     * Process the provided terrain map and
     * generate the proper internal reppresentation
     */
    std::size_t x_size = map[0].size();
    for( std::size_t y{ 0 } ; y < map.size() ; ++y )
    {
        if( x_size != map[ y ].size() ) {
            //Should be equal for all,the map should be a quad
            ERR("The provided terrain map is not a quad!");
            return false;
        }
        for( std::size_t x{ 0 } ; x < map[y].size() ; ++x )
        {
            lot_pointer new_lot = std::make_shared< terrain_lot >();
            new_lot->terrain_id = map[ y ][ x ];
            new_lot->position = glm::vec2(x - central_lot.x, y - central_lot.y );
            new_lot->model_matrix = get_lot_model_matrix( new_lot->position );
            new_lot->default_color = terrain_container[ new_lot->terrain_id ].default_color;
             /*
             * Is the lot model available?
             */
            auto it = terrain_container.find( new_lot->terrain_id );
            if( it == terrain_container.end() )
            {
                WARN1("The model for the terrain ID: ",
                      new_lot->terrain_id," is not loaded..");
            } else {
                new_lot->model = it->second;
                new_lot->height = it->second.low_res_model->get_model_height();
            }
            long lot_idx = get_position_idx( new_lot->position );
            if( terrain_map.find( lot_idx ) != terrain_map.end() ) {
                ERR("Attempt to add twice a lot at the same idx: ",
                    lot_idx, " number of loaded lots: ", terrain_map.size() );
            } else {
                new_lot->rendr_id = renderer->add_renderable( new_lot );
                new_lot->set_rendering_state( renderer::renderable_state::rendering_enabled );
                terrain_map[ get_position_idx( new_lot->position ) ] = new_lot;
            }
        }
    }
    return true;
}

/*
 * The 'dir' vector specify the proper ray cast
 * of the mouse when moving over the terrain
 */
void terrains::mouse_hover(const types::ray_t &dir)
{
    unselect_highlighted_lot();
    glm::vec2 target = get_lot_position( dir );
    select_highlighted_lot( target );
}

void terrains::set_view_center(const glm::vec2 &pos,
                               const GLfloat distance)
{
    field_of_view_distance = distance * lot_size;
    /*
     * view_center are the coordinates of the
     * lot at the 'center' of the eye view
     */
    view_center = pos;
    view_center += 1.0f;
    view_center /= lot_size;
    view_center = glm::floor( view_center );
    long visible_obj_count{ 0 };
    //Recalculate the visible lots
    for( auto& entry : terrain_map ) {
        auto lot = entry.second;
        if( glm::distance( view_center, lot->position ) <= distance ){
            lot->visible = true;
            ++visible_obj_count;
        } else {
            lot->visible = false;
        }
    }
    update_rendr_quality( visible_obj_count );
}

glm::mat4 terrains::get_lot_model_matrix(const glm::vec2 &pos) const
{
    glm::mat4 model;
    return glm::translate( model,
                           glm::vec3( pos.x * lot_size,
                                      pos.y * lot_size,
                                      0.0) );
}

glm::mat4 terrains::get_lot_top_model_matrix(const glm::vec2 &pos) const
{
    glm::mat4 model = get_lot_model_matrix( pos );
    auto it = terrain_map.find( get_position_idx( pos ) );
    if( it == terrain_map.end() ) {
        ERR("Not able to find the terrain lot at position ", pos);
        return {};
    }
    return glm::translate( model,
                           glm::vec3(0.0,
                                     0.0,
                                     it->second->height));
}

glm::vec2 terrains::get_lot_position( const types::ray_t &dir ) const
{
    /*
     * Find the position with Z=0, we simplify thing
     * thing by ignoring the Z of the models
     */
    glm::vec3 position = dir.first;
    GLfloat l = 0,
            r = 1024; //Hopefully is big enough!
    while( l < r ) {
        GLfloat mid = ( r + l ) / 2;
        position = dir.first + dir.second * mid;
        if( glm::abs( position.z ) <= 0.00001 ) {
            //Looks like 0 :)
            break;
        }
        if( position.z > 0 ) {
            l = mid + 0.0001;
        } else {
            r = mid - 0.0001;
        }
    }
    position.z = 0;
    /*
     * There's a weird offset that we need to fix,
     * also the size of each lot is lot_size * lot_size
     */
    position += 1.0f;
    position /= lot_size;
    position = glm::floor( position );
    glm::vec2 lot_position( position.x, position.y );
    if( false == is_a_valid_position( lot_position ) ) {
        //Not lots at this position, set to inf,inf
        lot_position.x = std::numeric_limits<GLfloat>::max();
        lot_position.y = std::numeric_limits<GLfloat>::max();
    }
    return lot_position;
}

bool terrains::is_a_valid_position(const glm::vec2 &pos) const
{
    return terrain_map.find( get_position_idx( pos ) ) != terrain_map.end();
}

glm::vec2 terrains::get_coord_origin() const
{
    return origins_lot;
}

GLfloat terrains::get_position_idx(const glm::vec2 &pos) const
{
    /*
     * Cantor Pairing function.
     * Make sure to not calculate the index
     * using negative values
     */
    const GLfloat x = pos.x + origins_lot.x + 5;
    const GLfloat y = pos.y + origins_lot.y + 5;
    return (1.0f/2.0f) * ( x + y ) * ( x + y + 1) + y;
}

void terrains::update_rendr_quality(long rendr_mesh_cnt)
{
    rendering_quality new_quality{ rendering_data.current_rendr_quality };
    if( rendr_mesh_cnt <= rendering_data.highres_shift_max_mesh ) {
        //Let's use highres models
        new_quality = rendering_quality::highres;
    } else if( rendr_mesh_cnt >= rendering_data.lowres_shift_min_mesh ) {
        //Let's use lowres models
        new_quality = rendering_quality::lowres;
    }
    if( new_quality != rendering_data.current_rendr_quality ) {
        rendering_data.current_rendr_quality = new_quality;
        LOG1("Switching to ",
             new_quality == rendering_quality::highres ? "highres" : "lowres"
             ," model quality. Mesh count: ",rendr_mesh_cnt);
    }
}

void terrains::unselect_highlighted_lot()
{
    highlighted_lot.x = std::numeric_limits<GLfloat>::max();
    highlighted_lot.y = std::numeric_limits<GLfloat>::max();
}

void terrains::select_highlighted_lot(const glm::vec2 &lot)
{
    highlighted_lot = lot;
}

/*
 * Return true if 'lot' should be highlighted
 */
bool terrains::is_highlighted(const glm::vec2 &lot) const
{
    return highlighted_lot.x == lot.x &&
            highlighted_lot.y == lot.y;
}

glm::vec3 terrains::highlight_lot_color(const glm::vec3 &color) const
{
    return color * 1.3f;
}

long terrains::generate_unique_id()
{
    long terrain_id{ -1 };
    if( used_ids.empty() ) {
        terrain_id = 1;
    } else {
        /*
         * std::set is sorted, pick the largest
         * one and generate a new unique ID
         */
        terrain_id = *used_ids.rbegin() + 1;
    }
    //Attemp to insert the new ID
    return used_ids.insert( terrain_id ).second ? terrain_id : -1;
}

void terrain_lot::prepare_for_render( shaders::shader_ptr& shader )
{
}

void terrain_lot::render(shaders::shader_ptr &shader)
{
    for( auto&& mesh : model.high_res_model->get_mesh() ) {
        mesh->render( &*shader );
    }
}

void terrain_lot::clean_after_render( shaders::shader_ptr &shader )
{

}


}
