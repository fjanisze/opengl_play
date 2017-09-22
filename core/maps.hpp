#ifndef CORE_MAPS_HPP
#define CORE_MAPS_HPP
#include <factory.hpp>
#include <types.hpp>
#include <memory>
#include <iostream>
#include "resources.hpp"
#include "terrains.hpp"

namespace core_maps {

class Map_lot
{
public:
    using pointer = std::shared_ptr< Map_lot >;
    using row_type = std::vector< pointer >;
    using container = std::vector< row_type >;
    Map_lot( const types::point position );

    Lot_config::pointer lot;
    const id_factory< Map_lot > id;
    const types::point position;
    graphic_terrains::Terrain_lot::pointer rendr_lot;

    /*
     * Those are the ID's of the units placed on this
     * map lot
     */
    std::set< types::id_type > units;

    bool is_traversable() const
    {
        return traversable_lot::yes == lot->current_specs.traversable;
    }
    /*
     * Pointers to the neighbourhood Lot's
     */
    std::vector< pointer > adjacent_lots;
    void add_adjacent( pointer adj_lot )
    {
        if ( nullptr != adj_lot ) {
            adjacent_lots.push_back( adj_lot );
        }
    }
};

/*
 * contains the functionality which calculate the shortest
 * path between two lots
 */
class Map_paths
{
    void bfs( Map_lot::pointer& root );
public:
    using path_elems = std::vector< Map_lot::pointer >;
    path_elems shortest( Map_lot::pointer& root,
                         Map_lot::pointer& target );
private:
    Map_lot::pointer target_lot;
    path_elems shortest_path;
    std::map< types::id_type, bool > visited;
    std::map< types::id_type, Map_lot::pointer > parents;
};

/*
 * This object reppresent a...MAP!
 */
class Map
{
    /*
     * It creates all the empty Map_lot for the map
     */
    void allocate_map();
    /*
     * Update the mapping from renderer lot information
     * to core lot information
     */
    void update_rendr_core_mapping( const graphic_terrains::terrain_map_t& ids_map );
public:
    using pointer = std::shared_ptr< Map >;
    Map( const size_t size );

    const id_factory< Map_lot > id;
    const size_t      size;
public:
    Map_lot::pointer get_lot( const types::point& position );
    Map_lot::pointer get_lot( types::id_type rendr_lot_id );
    Map_paths paths;
private:
    Map_lot::container map_data;
    /*
     * This mapping is needed to find which core lot
     * is related with a terrain lot as seen by the
     * graphic renderer.
     */
    std::map< types::id_type, Map_lot::pointer > rendr_to_core_mapping;
    friend class Maps;
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
