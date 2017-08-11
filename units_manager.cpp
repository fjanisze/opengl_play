#include <units_manager.hpp>

namespace graphic_units {

void Movements::change_unit_heading(
    Unit::pointer& unit,
    const Terrain_lot::pointer& target
)
{
    const GLfloat new_heading = calculate_heading(
                                    unit->rendering_data.position,
                                    target->rendering_data.update_pos_from_model_matrix() );
    unit->rendering_data.model_matrix = glm::rotate(
                                            unit->rendering_data.model_matrix,
                                            new_heading - unit->rendering_data.heading,
                                            glm::vec3( 0.0f, 0.0f, 1.0f ) );
    unit->rendering_data.heading = new_heading;
}

void Movements::place_unit_on_lot(
    Unit::pointer& unit,
    const graphic_terrains::Terrain_lot::pointer& lot )
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
                                            ) );
    /*
     * Account for the heading as well
     */
    unit->rendering_data.model_matrix = glm::rotate(
                                            unit->rendering_data.model_matrix,
                                            unit->rendering_data.heading,
                                            glm::vec3( 0.0f, 0.0f, 1.0f ) );
    unit->rendering_data.update_pos_from_model_matrix();
}

GLfloat Movements::calculate_heading(
    const types::point& source,
    const types::point& target ) const
{
    if ( source == target ) {
        return 0.0f;
    }
    return std::atan2( target.y - source.y,
                       target.x - source.x ) + glm::half_pi<GLfloat>();
}

Move_processor::Move_processor(
    Unit::pointer unit,
    Terrain_lot::pointer lot ) :
    unit{ unit },
    target_lot{ lot },
    status{ Move_processor::moving_status::READY },
    original_position{ unit->rendering_data.position }
{
    LOG2( "Created, ID:", id, "! target unitID:", unit->id,
          ", target lot:", lot->id );
    const types::point from = unit->rendering_data.position;
    const types::point to = glm::vec3(
                                lot->rendering_data.position.x,
                                lot->rendering_data.position.y,
                                lot->altitude );
    distance = glm::distance( from, to );
    direction = glm::normalize( to - from );
    LOG2( "Distance: ", distance,
          ", Direction: ", direction,
          ", from:", from,
          ", to:", to );
    heading = movement_impl.calculate_heading( from, to );
}

Move_processor::~Move_processor()
{
    LOG2( "Destroyed, ID:", id );
}

Move_processor::moving_status Move_processor::get_status() const
{
    return status;
}

void Move_processor::start()
{
    start_time = std::chrono::duration_cast< std::chrono::microseconds >
                 (
                     std::chrono::high_resolution_clock::now().time_since_epoch()
                 );
    arrival_time = std::chrono::duration_cast< std::chrono::microseconds >
                   (
                       start_time + std::chrono::seconds(
                           static_cast< long int >( glm::ceil( distance / 4.0f ) )
                       )
                   );
    status = Move_processor::moving_status::MOVING;
    LOG2( "Movement unitID:", unit->id,
          ", start:", start_time.count(),
          ", arrival:", arrival_time.count(),
          ", distance:", distance );
}


Move_processor::moving_status Move_processor::step(
    const types::timestamp& current
)
{
    /*
     * The progress variable is needed to calculate at which
     * point of the path the unit is
     */
    const GLfloat progress = movement_impl.speed_function(
                                 static_cast<GLfloat>( ( current - start_time ).count() ) /
                                 static_cast<GLfloat>( ( arrival_time - start_time ).count() )
                             );

    const glm::vec3 new_dir = direction * progress * distance;
    const glm::vec3 new_pos = original_position + new_dir;
    glm::mat4 model;

    unit->rendering_state.set_disable();
    /*
     * Set the position and heading of the unit
     */
    unit->rendering_data.model_matrix = glm::translate(
                                            model,
                                            new_pos );

    unit->rendering_data.model_matrix = glm::rotate(
                                            unit->rendering_data.model_matrix,
                                            heading,
                                            glm::vec3( 0.0f, 0.0f, 1.0f ) );

    if ( current > arrival_time ) {
        /*
         * Movement completed
         */
        LOG2( "Complete! ID:", id );
        status = moving_status::COMPLETED;
        unit->rendering_data.heading = heading;
        movement_impl.place_unit_on_lot( unit, target_lot );
    }
    unit->rendering_state.set_enable();
    return status;
}


Units_movement_processor::Units_movement_processor(
    Units_data_container& container
) :
    units_container{ container }
{

}

bool Units_movement_processor::teleport(
    types::id_type unit_id,
    Terrain_lot::pointer target_lot )
{
    auto unit_info = units_container.find( unit_id );
    if ( nullptr == unit_info ) {
        return false;
    }
    unit_info->unit->rendering_state.set_disable();
    /*
     * Remove the unit from it's current location and place it
     * in the new lot.
     */
    if ( nullptr != unit_info->location ) {
        unit_info->location->units->remove( unit_info->unit );
    } else {
        LOG3( "Unit ID:", unit_id, ", do not have a location." );
    }
    mov_impl.change_unit_heading( unit_info->unit,
                                  target_lot );
    target_lot->units->add( unit_info->unit );
    unit_info->location = target_lot;
    mov_impl.place_unit_on_lot( unit_info->unit,
                                target_lot );
    unit_info->unit->rendering_state.set_enable();
    LOG1( "Unit ID:", unit_id, ", is now on position:",
          unit_info->unit->rendering_data.position );
    return true;
}

bool Units_movement_processor::multiple_teleport(
    const std::vector<types::id_type>& units,
    Terrain_lot::pointer& target_lot )
{
    LOG3( "Attempt to teleport ", units.size(), " units to lot ID:",
          target_lot->id );
    bool ret{ true };
    //Teleport them all..
    for ( auto&& unit : units ) {
        ret = teleport( unit, target_lot ) && ret;
    }
    return ret;
}

bool Units_movement_processor::move(
    types::id_type unit_id,
    Terrain_lot::pointer target_lot )
{
    LOG1( "Request to move unitID:", unit_id,
          " to lotID:", target_lot->id );

    auto unit_info = units_container.find( unit_id );
    if ( nullptr == unit_info ) {
        return false;
    }

    if ( unit_info->movement != nullptr ) {
        WARN1( "UnitID:", unit_id, " already moving!" );
        return false;
    }

    if ( unit_info->location == target_lot ) {
        LOG1( "Unit ", unit_id, " is already at location ",
              target_lot->id );
        return false;
    }

    unit_info->location->units->remove( unit_info->unit );
    unit_info->movement = factory< Move_processor >::create(
                              unit_info->unit,
                              target_lot );
    unit_info->location = target_lot;

    start_movement( unit_info );
}

bool Units_movement_processor::multiple_move(
    const std::vector<types::id_type>& units,
    Terrain_lot::pointer& target_lot )
{
    LOG3( "Attempt to move ", units.size(), " units to lot ID:",
          target_lot->id );
    bool ret{ true };
    //Move them all..
    for ( auto&& unit : units ) {
        ret = move( unit, target_lot ) && ret;
    }
    return ret;
}

void Units_movement_processor::process_movements()
{
    if ( in_movement.empty() ) {
        return;
    }
    const types::timestamp current_time =
        std::chrono::duration_cast< std::chrono::microseconds > (
            std::chrono::high_resolution_clock::now().time_since_epoch()
        );
    for ( auto&& unit : in_movement ) {
        if ( unit->step( current_time ) == Move_processor::moving_status::COMPLETED ) {
            auto unit_info = units_container.find_nofail( unit->moving_unit_id() );
            /*
             * Cleanup the 'movement' variable in the Unit_info to make
             * sure that future movements are allowed
             */
            unit_info->movement = nullptr;
        }
    }
    in_movement.erase( std::remove_if( in_movement.begin(),
                                       in_movement.end(),
    [ this ]( const Move_processor::pointer & ptr ) {
        return ptr->get_status() == Move_processor::moving_status::COMPLETED;
    } ),
    in_movement.end() );
}

void Units_movement_processor::start_movement(
    Unit_info::pointer unit
)
{
    LOG2( "Starting movement for unitID:", unit->unit->id );
    if ( unit->movement == nullptr ) {
        PANIC( "Requested to start unit movement, but no movement data available!" );
    }
    in_movement.push_back( unit->movement );

    unit->movement->start();

    LOG2( "Number of unit in movement: ", in_movement.size() );
}

Units::Units( renderer::Core_renderer_proxy renderer ) :
    renderer{ renderer },
    movement_processor( units_container )
{
    LOG3( "Loading ", internal::units.size(), " unit models!" );
    for ( auto&& unit : internal::units ) {
        Unit_model::pointer model = factory< Unit_model >::create(
                                        unit
                                    );

        available_models.push_back( model );
    }
    LOG3( "Completed! Amount of available units: ",
          available_models.size() );
}

Unit_model_data::container Units::buildable_units()
{
    return internal::units;
}

Unit::pointer Units::create_unit( uint64_t id )
{
    LOG3( "Requested to create a new unit, ID:", id );
    auto model = find_model( id );
    if ( nullptr == model ) {
        ERR( "Not able to find the model with ID ", id );
        return nullptr;
    }
    Unit::pointer new_unit = factory< Unit >::create(
                                 model
                             );
    new_unit->rendering_state.set_disable();
    renderer.add_renderable( new_unit );
    units_container.add( new_unit );
    LOG3( "New unit created, unit ID:",
          new_unit->id,
          ", total amount of units: ", units_container.size() );
    return new_unit;
}

bool Units::place_unit( Unit::pointer unit,
                        graphic_terrains::Terrain_lot::pointer lot )
{
    LOG2( "Placing unit ", unit->id,
          " on lot ", lot->id, ", at position:",
          lot->rendering_data.position );
    if ( false == movement_processor.teleport( unit->id, lot ) ) {
        WARN2( "Not possible to place, teleport failed!" );
        return false;
    }
    return true;
}

Units_movement_processor& Units::movements()
{
    return movement_processor;
}

Unit_model::pointer Units::find_model( const uint64_t id )
{
    for ( auto&& model : available_models ) {
        if ( model->model_data.def.id == id ) {
            return model;
        }
    }
    return nullptr;
}

Units_data_container::Units_data_container()
{
    LOG3( "New Units_movement_data, ID:", id );
}

Unit_info::pointer Units_data_container::add( const Unit::pointer unit )
{
    LOG1( "Adding new unit, ID:", unit->id,
          " container ID:", this->id );
    const auto info = data.find( id );
    if ( info != data.end() ) {
        LOG1( "Not able to add, unit already present!" );
        return nullptr;
    }
    auto new_info = factory< Unit_info >::create();
    new_info->unit = unit;
    data[ unit->id ] = new_info;
    return new_info;
}

Unit_info::pointer Units_data_container::find( const types::id_type id )
{
    const auto it = data.find( id );
    if ( it == data.end() ) {
        WARN2( "UnitID:", id, " not found!" );
        return nullptr;
    }
    return it->second;
}

Unit_info::pointer Units_data_container::find_nofail( const types::id_type id )
{
    auto ptr = find( id );
    if ( ptr == nullptr ) {
        PANIC( "Not able to find the requested unit ID:", id,
               " in container ID:", this->id );
    }
    return ptr;
}

std::size_t Units_data_container::size() const
{
    return data.size();
}

}
