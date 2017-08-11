#ifndef CORE_MAPS_HPP
#define CORE_MAPS_HPP
#include <factory.hpp>
#include <types.hpp>
#include <memory>
#include "resources.hpp"
#include "terrains.hpp"

namespace core_maps {

enum class terrain_type {
    grass,
    mountain,
    forest,
};

enum class traversable_lot {
    yes,
    no
};

/*
 * Characteristics of a terrain lot, if units can move over it,
 * if there's a penalty for the move etc.
 */
struct Terrain_lot_spec {
    traversable_lot traversable;
};

/*
 * Definition of a terrain lot, include all the information
 * needed to draw the lot
 */
struct Terrain_lot_def {
    const resources::Model_resource_def& model_def;
    const std::string    name;

    const terrain_type   type;
    const Terrain_lot_spec specs;

    const std::string    desc;
};

struct Lot_specs {
    using pointer = std::shared_ptr< Lot_specs >;
    //Those are the default values for the lot
    const Terrain_lot_def def;
    //Those are the current values
    Terrain_lot_spec current;

    Lot_specs( const Terrain_lot_def definition ) :
        def{ definition },
        current{ definition.specs }
    {}
};

class Map_lot
{
public:
    using pointer = std::shared_ptr< Map_lot >;
    using row_type = std::vector< pointer >;
    using container = std::vector< row_type >;
    Map_lot( const types::point position );

    Lot_specs::pointer specification;
    const id_factory< Map_lot > id;
    const types::point position;
};

/*
 * This object reppresent a..MAP!
 */
class Map
{
    void allocate_map();
public:
    using pointer = std::shared_ptr< Map >;
    Map( const size_t size );

    const id_factory< Map_lot > id;
    const size_t      size;
    Map_lot::container map_data;
};

class Maps
{
    void assign_random_lots( Map::pointer& map );
    /*
     * This function looks for the terrain definition in the
     * global array terrain_definitions
     */
    const Terrain_lot_def& find_terrain_definition( const types::id_type id );
public:
    Maps( graphic_terrains::Terrains::pointer terrains );
    Map::pointer create_random_map( const size_t size );
private:
    graphic_terrains::Terrains::pointer terrains;
    std::vector< types::id_type > all_terrain_ids;
    std::map< types::id_type, Map::pointer > maps;
};

} //core_maps

#endif //CORE_MAPS_HPP
