#include <headers.hpp>
#include <units.hpp>
#include <terrains.hpp>

namespace game_units
{

/*
 * Object which handle the creation, destruction
 * of units. Units placement, movement etc..
 */
class Units
{
public:
    using pointer = std::shared_ptr< Units >;
    Units();
    /*
     * Return the set of units that Units is able
     * to build or recognize
     */
    Unit_model_data::container buildable_units();

    Unit::pointer create_unit( uint64_t id );
private:
    /*
     * Those are the units we ca use for
     * building, each call to create_unit
     * must be for one of the available units
     */
    Unit_model::container available_models;
    Unit_model::pointer find_model( const uint64_t id );
    /*
     * Those are the units which were actually
     * create by the user (possibly placed somewhere).
     */
    Unit::container units;
};

}
