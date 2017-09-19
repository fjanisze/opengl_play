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

    Unit_config::pointer unit;
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
    Units( graphic_units::Units::pointer );
    Unit::pointer create( const types::id_type id );
    /*
     * Place a unit on a given lot
     */
    bool place( Unit::pointer& unit, core_maps::Map_lot::pointer& lot );
private:
    /*
     * Pointer to the graphical reppresentation
     * of the units
     */
    graphic_units::Units::pointer rendr_units;
    /*
     * Contains all the units
     */
    std::map< types::id_type, Unit::pointer > units;
};

}

#endif //CORE_UNITS_HPP
