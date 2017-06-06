#include <units_manager.hpp>

namespace game_units
{

Units_movement_processor::Units_movement_processor(
        Units_movement_data &container
        ) :
    units_container{ container }
{

}

bool Units_movement_processor::teleport(
        types::id_type unit_id,
        Terrain_lot::pointer target_lot )
{
    auto unit_info = units_container.find( unit_id );
    if( unit_info != nullptr ) {
        unit_info->target_unit->rendering_state.set_disable();
        /*
         * Remove the unit from it's current location and place it
         * in the new lot.
         */
        if( nullptr != unit_info->location_lot ) {
            unit_info->location_lot->units->remove( unit_info->target_unit );
        } else {
            LOG3("Unit ID:", unit_id,", do not have a location.");
        }
        target_lot->units->add( unit_info->target_unit );
        unit_info->location_lot = target_lot;
        update_unit_position( unit_info->target_unit,
                              target_lot );
        unit_info->target_unit->rendering_state.set_enable();
        LOG1("Unit ID:",unit_id,", is now on position:",
             unit_info->target_unit->rendering_data.position);
    } else {
        WARN2("Unable to move unit with ID:", unit_id,
             ", Unit not found!");
        return false;
    }
    return true;
}

bool Units_movement_processor::multiple_teleport(
        const std::vector<types::id_type> &units,
        Terrain_lot::pointer &target_lot )
{
    LOG3("Attempt to move ", units.size()," units to lot ID:",
         target_lot->id);
    for( auto&& unit : units ) {
        if( nullptr == units_container.find( unit ) ) {
            //Not recognized unit, not able to move
            WARN1("Unit ID:",unit," not found!");
            return false;
        }
    }
    bool ret{ true };
    //Move them all..
    for( auto&& unit :units ) {
        ret = ret && teleport( unit,
                   target_lot );
    }
    return ret;
}

void Units_movement_processor::update_unit_position(
        Unit::pointer &unit,
        const game_terrains::Terrain_lot::pointer &lot )
{
    const glm::mat4 lot_mod_matx = lot->rendering_data.model_matrix;
    unit->rendering_data.model_matrix = lot_mod_matx;
    /*
     * In order to avoid units to be 'inside'
     * the terrain model, we need to translate the
     * model matrix of an amount equal to the 'altitude'
     * of the terrain'
     */
    unit->rendering_data.model_matrix = glm::translate(
                unit->rendering_data.model_matrix,
                glm::vec3(
                    0.0,
                    0.0,
                    lot->altitude
                    ));
    unit->rendering_data.update_pos_from_model_matrix();
}


Units::Units( renderer::Core_renderer_proxy renderer ) :
    renderer{ renderer },
    movement_processor( units_container )
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
    new_unit->rendering_state.set_disable();
    renderer.add_renderable( new_unit );
    units_container.add( new_unit );
    LOG3("New unit created, unit ID:",
         new_unit->id,
         ", total amount of units: ", units_container.size());
    return new_unit;
}

bool Units::place_unit( Unit::pointer unit,
                       game_terrains::Terrain_lot::pointer lot )
{
    LOG2( "Placing unit ", unit->id,
          " on lot ",lot->id, ", at position:",
          lot->rendering_data.position);
    if( false == movement_processor.teleport( unit->id, lot ) ) {
        WARN2("Not possible to place, teleport failed!");
        return false;
    }
    return true;
}

Units_movement_processor &Units::movements()
{
    return movement_processor;
}

Unit_model::pointer Units::find_model(const uint64_t id)
{
    for( auto&& model :available_models ) {
        if( model->model_data.id == id )
            return model;
    }
    return nullptr;
}

Unit_info::pointer Units_movement_data::add( const Unit::pointer unit )
{
    LOG1("Adding new unit, ID:", unit->id );
    const auto info = find( unit->id );
    if( info != nullptr ) {
        LOG1("Not able to add, unit already present!");
        return nullptr;
    }
    auto new_info = factory< Unit_info >::create();
    new_info->target_unit = unit;
    data[ unit->id ] = new_info;
    return new_info;
}

Unit_info::pointer Units_movement_data::find( const types::id_type id )
{
    const auto it = data.find( id );
    if( it == data.end() ) {
        return nullptr;
    }
    return it->second;
}

std::size_t Units_movement_data::size() const
{
    return data.size();
}


}
