#ifndef CORE_UNITS_HPP
#define CORE_UNITS_HPP
#include <types.hpp>
#include <string>
#include "resources.hpp"

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
    const int num_of_movements;
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

/*
 * Unit creation/destruction, placement on the map &c.
 */
class Units
{
public:
    Units();
};

}

#endif //CORE_UNITS_HPP
