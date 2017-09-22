#ifndef CORE_UNITS_HPP
#define CORE_UNITS_HPP
#include <types.hpp>
#include <string>
#include "resources.hpp"
#include "../units_manager.hpp"
#include "maps.hpp"

namespace core_units {

/*
 * Reppresentation of a single unit
 */
class Unit
{
public:
    using pointer = std::shared_ptr< Unit >;
    Unit( const Unit_def& definition );

    Unit_config::pointer config;
    types::point position;
    const id_factory< Unit > id;

    graphic_units::Unit::pointer rendr_unit;
};

/*
 * Unit creation/destruction, placement on the map &c.
 */
class Units
{
    const Unit_def& find_unit_definition( const types::id_type id );
public:
    using pointer = std::shared_ptr< Units >;
    Units( graphic_units::Units::pointer,
           core_maps::Map::pointer map );
    Unit::pointer create( const types::id_type id );
    /*
     * Place a unit on a given lot
     */
    bool place( Unit::pointer& unit, core_maps::Map_lot::pointer& lot );
    /*
     * A unit was selected, trigger the proper action which
     * in the case of movable units is to show how far the unit can
     * be moved
     */
    void select( Unit::pointer& unit );
    void select( const types::id_type renderable_id );
    void unselect();
private:
    /*
     * The map where those units are going to be
     * placed, moved &c
     */
    core_maps::Map::pointer game_map;
    /*
     * Pointer to the graphical reppresentation
     * of the units
     */
    graphic_units::Units::pointer rendr_units;
    /*
     * Contains all the units
     */
    std::map< types::id_type, Unit::pointer > units;
    /*
     * Map renderable to a unit, this is helpful for example
     * when picking models to check whether the model
     * is a unit..
     */
    std::map< types::id_type, Unit::pointer > rendr_to_unit;
    /*
     * Those are the terrains affected by the selection,
     * when unselecting make sure to dehighlight them
     */
    std::vector< graphic_terrains::Terrain_lot::pointer > affected_by_selection;
};

}

#endif //CORE_UNITS_HPP
