#include "maps.hpp"
#include <logger/logger.hpp>

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

Maps::Maps()
{
    LOG3( "Created!" );
    if ( log_inst.current_log_level() <= logging::severity_type::debug0 ) {
        for ( const auto& elem : terrain_definitions ) {
            LOG0( "Lot ID:", elem.model_def.id, ", model name:", elem.model_def.pretty_name,
                  ", model paths:", elem.model_def.model_path, ",",
                  elem.model_def.high_res_model_path );

        }
    }
}

}
