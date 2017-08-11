#include "units.hpp"
#include <logger/logger.hpp>
#include "resources.hpp"

namespace core_units {

const Unit_def units_definitions[] = {
    {
        resources::unit_poldek_def,
        "Poldek",
        unit_type::small_vehicle,
        {
            2 //Number of movements
        },
        "The best car on the planet!",
    }
};

Units::Units( graphic_units::Units::pointer units ) :
    units{ units }
{
    LOG3( "Created!" );
    for ( const auto& elem : units_definitions ) {
        LOG0( "Unit ID:", elem.model_def.id, ", model name:", elem.model_def.pretty_name,
              ", model paths:", elem.model_def.model_path, ",",
              elem.model_def.high_res_model_path );

    }
}

} //core_units
