#include <units_manager.hpp>

namespace game_units
{

Units::Units()
{
    LOG3("Loading ", internal::units.size()," unit models!");
    for( auto&& unit : internal::units )
    {
        Unit_model::pointer model = factory< Unit_model >::create(
                    unit
                    );

        available_models.push_back( model );
    }
    LOG3("Completed! Amount of available units: ",
         available_models.size() );
}

Unit_model_data::container Units::buildable_units()
{
    return internal::units;
}

Unit::pointer Units::create_unit( uint64_t id )
{
    LOG3("Requested to create a new unit, ID:",id);
    auto model = find_model( id );
    if( nullptr == model ) {
        ERR("Not able to find the model with ID ",id);
        return nullptr;
    }
    Unit::pointer new_unit = factory< Unit >::create(
                model
                );
    units->place_unit( new_unit );
    LOG3("New unit created, unit ID:", new_unit->id,
         ", total amount of units: ", units->size());
    return new_unit;
}

Unit_model::pointer Units::find_model(const uint64_t id)
{
    for( auto&& model :available_models ) {
        if( model->model_data.id == id )
            return model;
    }
    return nullptr;
}

}
