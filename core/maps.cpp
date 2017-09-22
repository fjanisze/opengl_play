#include "maps.hpp"
#include <logger/logger.hpp>
#include <random>

namespace core_maps {

/*
 * Hardcoded definitions of all the available terrains
 */
const Terrain_lot_def terrain_definitions[] = {
    {
        resources::terrain_grass1_def,
        "Grass",
        terrain_type::grass,
        {
            traversable_lot::yes
        },
        "Some grass, without too much hills"
    },
    {
        resources::terrain_grass2_def,
        "Grass 2",
        terrain_type::grass,
        {
            traversable_lot::yes
        },
        "Some grass, with some hills"
    },
    {
        resources::terrain_forest_def,
        "Forest",
        terrain_type::grass,
        {
            traversable_lot::yes
        },
        "A forest.."
    },
    {
        resources::terrain_mountain_def,
        "Mountain",
        terrain_type::grass,
        {
            traversable_lot::no
        },
        "A big solid mountain"
    }
};

Map_lot::Map_lot( const types::point position ):
    position{ position }
{
}

void Map::allocate_map()
try
{
    map_data.resize( size );
    for ( int y{0}; y < size; ++y ) {
        map_data[ y ].resize( size );
        for ( int x{0}; x < size; ++x ) {
            map_data[ y ][ x ] = factory< Map_lot >::create(
                                     types::point( x, y, 0 ) );
        }
    }
} catch ( std::exception& ex )
{
    ERR( "Exception raised during map allocation: ", ex.what() );
    PANIC( "Unable to recover!" );
}

Map::Map( const size_t size ) :
    size{ size }
{
    LOG3( "New map size: ", size, ",mapID:", id );
    allocate_map();
}

Map_lot::pointer Map::get_lot( types::point&& position )
{
    return map_data[ position.y ][ position.x ];
}

Map_lot::pointer Map::get_lot( types::id_type rendr_lot_id )
{
    auto lot = rendr_to_core_mapping.find( rendr_lot_id );
    if ( lot != rendr_to_core_mapping.end() ) {
        return lot->second;
    }
    PANIC( "Not able to find the lot with rendrID:", rendr_lot_id );
    return nullptr;
}

void Map::update_rendr_core_mapping(
    const graphic_terrains::terrain_map_t& ids_map )
{
    LOG3( "Updating the mapping!" );
    for ( int y{ 0 }; y < size; ++y ) {
        for ( int x{ 0 } ; x < size ; ++x ) {
            rendr_to_core_mapping[ ids_map[ y ][ x ] ] = map_data[ y ][ x ];
        }
    }
}

/*
 * Given the list of available terrain lots, assign to
 * each of the lots of the map a random terrain.
 */
void Maps::assign_random_lots( Map::pointer& map )
{
    LOG3( "Assigning random lots to mapID:", map->id );
    std::random_device rd;
    std::mt19937_64 eng( rd() );
    std::uniform_int_distribution<long> dist( 0,
            all_terrain_ids.size() - 1 );
    for ( int y{ 0 } ; y < map->size; ++y ) {
        for ( int x{ 0 }; x < map->size; ++x ) {
            types::id_type terrain_id_idx = dist( eng );
            auto terrain = find_terrain_definition( all_terrain_ids[ terrain_id_idx ] );
            auto map_data = map->map_data[ y ][ x ];

            map_data->lot = factory< Lot_config >::create( terrain );
        }
    }
}

/*
 * Given the provided ID return the terrain definition
 * for that ID or PANIC.
 */
const Terrain_lot_def& Maps::find_terrain_definition(
    const types::id_type id )
{
    for ( auto& elem : terrain_definitions ) {
        if ( elem.model_def.id == id ) {
            return elem;
        }
    }
    PANIC( "terrain definition for terrain ID:", id, " not found!" );
}

Maps::Maps( graphic_terrains::Terrains::pointer terrains ) :
    terrains{ terrains }
{
    LOG3( "Created!" );
    /*
     * Load to the graphic terrains handler all the available
     * terrains
     */
    for ( const auto& elem : terrain_definitions ) {
        LOG0( "Lot ID:", elem.model_def.id, ", model name:",
              elem.model_def.pretty_name,
              ", model paths:", elem.model_def.model_path, ",",
              elem.model_def.high_res_model_path );
        terrains->load_terrain( elem.model_def.model_path,
                                glm::vec4( 1.0 ),
                                elem.model_def.id );
        terrains->load_highres_terrain( elem.model_def.high_res_model_path,
                                        elem.model_def.id );
        if ( std::find( all_terrain_ids.begin(), all_terrain_ids.end(),
                        elem.model_def.id ) != all_terrain_ids.end() ) {
            PANIC( "Multiple terrains definition share the same ID:",
                   elem.model_def.id, "!" );
        } else {
            all_terrain_ids.push_back( elem.model_def.id );
        }
    }
}

Map::pointer Maps::create_random_map( const size_t size )
{
    LOG3( "Creating a random map of size: ", size );
    if ( size < 8 || size > 128 ) { //Hardcoded is better! :)
        PANIC( "The size of the map must be within the range 8<=size<=128" );
        return nullptr;
    }
    auto new_map = factory<Map>::create( size );
    assign_random_lots( new_map );
    const glm::vec2 central_lot( size / 2,
                                 size / 2 );
    /*
     * Generate the terrain_map needed by the graphic
     * renderer to render the map and load it to the
     * renderer itself
     */
    graphic_terrains::terrain_map_t terrain_map;
    terrain_map.resize( size );
    for ( int y{ 0 }; y < size; ++y ) {
        terrain_map[ y ].resize( size );
        for ( int x{ 0 } ; x < size ; ++x ) {
            terrain_map[ y ][ x ] = new_map->map_data[ y ][ x ]->lot->def.model_def.id;
        }
    }
    terrains->load_terrain_map( terrain_map,
                                2,
                                central_lot );
    LOG3( "Terrain map loaded, setting up the Terrain_lot pointers!" );
    /*
     * Assign to each Map_lot it's counterpart from the
     * renderer structures
     */
    for ( int y{ 0 }; y < size; ++y ) {
        for ( int x{ 0 } ; x < size ; ++x ) {
            auto& map_lot = new_map->map_data[ y ][ x ];
            map_lot->rendr_lot = terrains->find_lot( glm::vec2( x, y ) - central_lot );
            if ( nullptr == map_lot->rendr_lot ) {
                PANIC( "Not able to set the Terrain_lot pointer for MapID:", map_lot->id,
                       " at position ", map_lot->position );
            }
        }
    }
    /*
     * load_terrain_map update the provided map with the ID's
     * of the lots for each map coordinate, we need those information
     * to map core operations with the graphics engine
     */
    new_map->update_rendr_core_mapping( terrain_map );

    maps[ new_map->id ] = new_map;
    LOG3( "New map creation completed, amount of maps:", maps.size() );
    return new_map;
}

}
