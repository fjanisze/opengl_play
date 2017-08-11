#ifndef CORE_UNITS_HPP
#define CORE_UNITS_HPP
#include <types.hpp>
#include <string>
#include "resources.hpp"
#include "../units_manager.hpp"

namespace core_units {

/*
 * One entry for each possible type of unit
 */
enum class unit_type {
    small_vehicle,
};

/*
 * Some specification details about the units
 */
struct Unit_specs {
    int num_of_movements;
};

/*
 * Definition of a unit, there should be a definition for
 * each available unit.
 */
struct Unit_def {
    const resources::Model_resource_def& model_def;
    const std::string    name;

    const unit_type      type;
    const Unit_specs     specs;

    const std::string    desc;
};

struct Unit_config {
    using pointer = std::shared_ptr< Unit_config >;
    const Unit_def def;

    Unit_specs current_specs;

    Unit_config( const Unit_def& definition ) :
        def{ definition },
        current_specs{ definition.specs }
    {}
};

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
};

/*
 * Unit creation/destruction, placement on the map &c.
 */
class Units
{
    const Unit_def& find_unit_definition( const types::id_type id );
public:
    Units( graphic_units::Units::pointer );
    Unit::pointer create( const types::id_type id );
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
