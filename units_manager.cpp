#include <units_manager.hpp>

namespace game_units
{

Units::Units( renderer::Core_renderer_proxy renderer ) :
    renderer{ renderer }
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

    units = factory< Units_container >::create();
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
    units->add_unit( new_unit );
    LOG3("New unit created, unit ID:", new_unit->id,
         ", total amount of units: ", units->size());
    return new_unit;
}

bool Units::place_unit(Unit::pointer unit,
                       game_terrains::Terrain_lot::pointer lot)
{
    LOG3( "Placing unit ", unit->id," on lot ",lot->id );
    /*
     * Make sure this unit is not enabled for
     * rendering, then attempt to add it to the lot
     * and enable it back for rendering
     */
    unit->rendering_state.disable();
    if( lot->units->add_unit( unit ) )
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
        renderer.add_renderable( unit );
        unit->rendering_state.enable();
    } else {
        WARN2("Not possible to place!");
    }
    return false;
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
