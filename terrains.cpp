#include <terrains.hpp>
#include <logger/logger.hpp>
#include <factory.hpp>

namespace graphic_terrains {

Terrains::Terrains( renderer::Core_renderer_proxy renderer ) :
    renderer{ renderer }
{
    LOG3( "Creating terrains::terrains" );
}

long Terrains::load_terrain( const std::string& model_filename,
                             const glm::vec4& color,
                             long terrain_id )
{
    LOG3( "Loading terrain model: ",
          model_filename,
          ". Provided ID: ",
          terrain_id );
    auto new_model = factory< models::model_loader >::create(
                         model_filename );
    if ( false == new_model->load_model() ) {
        PANIC( "Not able to load the selected terrain!" );
    }
    if ( terrain_id < 0 ) {
        terrain_id = ids< Terrain_lot >::create();
    }
    auto it = terrain_map.find( terrain_id );
    if ( it != terrain_map.end() ) {
        ERR( "A terrain lot with ID ", terrain_id, " already Exist!" );
        terrain_id = -1;
    }
    if ( terrain_id > 0 ) {
        terrain_container[ terrain_id ].low_res_model = new_model;
        terrain_container[ terrain_id ].high_res_model = new_model;
        terrain_container[ terrain_id ].default_color = color;
        LOG3( "New terrain loaded, id: ", terrain_id,
              ". Amount of terrains: ", terrain_container.size() );
    } else {
        ERR( "The provided terrain ID is already in use!" );
        terrain_id = -1;
    }

    return terrain_id;
}

long Terrains::load_highres_terrain( const std::string& model_filename,
                                     long terrain_id )
{
    LOG3( "Loading highres terrain model: ",
          model_filename,
          ". Provided ID: ",
          terrain_id );
    auto it = terrain_container.find( terrain_id );
    if ( it == terrain_container.end() ) {
        ERR( "Not able to find the terrain with ID: ", terrain_id,
             ". Not possible to load the highres model.." );
        return -1;
    }
    auto new_model = factory< models::model_loader >::create(
                         model_filename );
    if ( false == new_model->load_model() ) {
        PANIC( "Not able to load the highres terrain!" );
    }
    it->second.high_res_model = new_model;
    return terrain_id;
}

bool Terrains::load_terrain_map( terrain_map_t& map,
                                 GLfloat lot_size,
                                 glm::vec2 central_lot )
{
    if ( lot_size <= 0 ) {
        ERR( "Lot size couldn't be zero!" );
        return false;
    }
    if ( map.empty() || map[0].empty() ) {
        ERR( "The provided terrain map is empty!" );
        return false;
    }
    this->lot_size = lot_size;
    origins_lot = central_lot;
    /*
     * Process the provided terrain map and
     * generate the proper internal reppresentation
     */
    std::size_t x_size = map[0].size();
    for ( std::size_t y{ 0 } ; y < map.size() ; ++y ) {
        if ( x_size != map[ y ].size() ) {
            //Should be equal for all,the map should be a quad
            ERR( "The provided terrain map is not a quad!" );
            return false;
        }
        for ( std::size_t x{ 0 } ; x < map[y].size() ; ++x ) {
            long model_id = map[ y ][ x ];
            /*
            * Is the lot model available?
            */
            auto it = terrain_container.find( model_id );
            if ( it == terrain_container.end() ) {
                PANIC( "The model for the terrain ID: ",
                       model_id, " is not loaded.." );
            }
            const glm::vec2 lot_position = glm::vec2( x - central_lot.x,
                                           y - central_lot.y );
            const long lot_idx = get_position_idx( lot_position );

            Terrain_lot::pointer new_lot = factory< Terrain_lot >::create(
                                               model_id,
                                               lot_position,
                                               it->second.low_res_model->get_model_height()
                                           );

            new_lot->rendering_data.model_matrix = get_lot_model_matrix(
                    new_lot->position );
            new_lot->rendering_data.update_pos_from_model_matrix();
            new_lot->rendering_data.default_color = terrain_container[
            new_lot->terrain_model_id ].default_color;
            new_lot->textures = it->second;

            if ( terrain_map.find( lot_idx ) != terrain_map.end() ) {
                ERR( "Attempt to add twice a lot at the same idx: ",
                     lot_idx, " number of loaded lots: ", terrain_map.size() );
            } else {
                renderer.add_renderable( new_lot );
                new_lot->rendering_state.set_enable();
                long idx = get_position_idx( new_lot->position );
                terrain_map[ idx ] = new_lot;
                rendr_id_to_idx[ new_lot->id ] = idx;
                /*
                 * Update the original map with the identifier
                 * for the newly created lot.
                 */
                map[ y ][ x ] = new_lot->id;
            }
        }
    }
    return true;
}

Terrain_lot::pointer Terrains::find_lot( const glm::vec2 coord )
{
    long pos_idx = get_position_idx( coord );
    auto it = terrain_map.find( pos_idx );
    if ( terrain_map.end() == it ) {
        ERR( "Lot at position ", coord, " not found!" );
        return nullptr;
    }
    return it->second;
}

Terrain_lot::pointer Terrains::find_lot(
    const renderer::Renderable::pointer& rendr
)
{
    auto it = rendr_id_to_idx.find( rendr->id );
    if ( it == rendr_id_to_idx.end() ) {
        return nullptr;
    }
    return terrain_map[ it->second ];
}

glm::mat4 Terrains::get_lot_model_matrix( const glm::vec2& pos ) const
{
    glm::mat4 model;
    return glm::translate( model,
                           glm::vec3( pos.x * lot_size,
                                      pos.y * lot_size,
                                      0.0 ) );
}

long Terrains::get_position_idx( const glm::vec2& pos ) const
{
    /*
     * Cantor Pairing function.
     * Make sure to not calculate the index
     * using negative values
     */
    const long x = pos.x + origins_lot.x + 5;
    const long y = pos.y + origins_lot.y + 5;
    return ( 1.0f / 2.0f ) * ( x + y ) * ( x + y + 1 ) + y;
}

Terrain_lot::Terrain_lot( const long model_id,
                          const glm::vec2 unique_position,
                          const GLfloat lot_altitude ) :
    terrain_model_id{ model_id },
    position{ unique_position },
    altitude{ lot_altitude }
{
    LOG0( "New lot created, ID:", id );
    units = factory< graphic_units::Units_container >::create();
}

bool Terrain_lot::render( )
{
    for ( auto&& mesh : textures.high_res_model->get_mesh() ) {
        if ( false == mesh->render( rendering_data.shader ) ) {
            return false;
        }
    }
    return true;
}

}
