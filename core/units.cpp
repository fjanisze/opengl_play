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
            1 //Number of movements
        },
        "The best car on the planet!",
    }
};

const Unit_def& Units::find_unit_definition( const types::id_type id )
{
    for ( auto& unit : units_definitions ) {
        if ( unit.model_def.id == id ) {
            return unit;
        }
    }
    PANIC( "unit definition for unit ID:", id, " not found!" );
}

Unit::Unit( const Unit_def& definition ) :
    position{ types::point( 0.0 ) }
{
    LOG3( "Creating a new unit, modelID:", definition.model_def.id,
          ",name:", definition.model_def.pretty_name,
          ", unitID:", id );
    config = factory< Unit_config >::create( definition );
}

Units::Units( graphic_units::Units::pointer units,
              core_maps::Map::pointer map ) :
    game_map{ map },
    rendr_units{ units }
{
    LOG3( "Created!" );
    for ( const auto& elem : units_definitions ) {
        LOG0( "Unit ID:", elem.model_def.id, ", model name:", elem.model_def.pretty_name,
              ", model paths:", elem.model_def.model_path, ",",
              elem.model_def.high_res_model_path );

    }
}

Unit::pointer Units::create( const types::id_type id )
{
    LOG3( "Attempting to create unit with modelID:", id );
    auto& unit_def = find_unit_definition( id );

    auto new_unit = factory< Unit >::create( unit_def );
    new_unit->rendr_unit = rendr_units->create_unit( id );
    if ( nullptr == new_unit->rendr_unit ) {
        ERR( "Creation failed!" );
        return nullptr;
    }
    units[ new_unit->id ] = new_unit;
    rendr_to_unit[ new_unit->rendr_unit->id ] = new_unit;
    LOG3( "Number of units created so far: ", units.size() );
    return new_unit;
}

bool Units::place( Unit::pointer& unit,
                   core_maps::Map_lot::pointer& lot )
{
    LOG3( "Attempting to place the UnitID:", unit->id,
          " on the LotID:", lot->id );
    if ( false == lot->is_traversable() ) {
        WARN1( "This lot is not traversable, unit placement failed" );
        return false;
    }
    auto result = lot->units.insert( unit->id );
    if ( false == result.second ) {
        ERR( "Unable to place the unit! (Already on that lot!)" );
        return false;
    }
    unit->position = lot->position;
    LOG0( "UnitID placed at position ", unit->position,
          ",turning ON rendering of the model" );
    return rendr_units->place_unit( unit->rendr_unit, lot->rendr_lot );
}

void Units::select( Unit::pointer& unit )
{
    if ( unit->config->current_specs.has_avail_movements() ) {
        LOG0( "Attempt to selected UnitID:", unit->id );
        /*
         * Show the reachable area for this unit,
         * highlight the accessible lots
         */
        const int mov_cnt{ unit->config->current_specs.num_of_movements };
        const int sx{ unit->position.x };
        const int sy{ unit->position.y };
        for ( int x{ sx - mov_cnt } ; x <= sx + mov_cnt ; ++x ) {
            for ( int y{ sy - mov_cnt } ; y <= sy + mov_cnt ; ++y ) {
                auto lot = game_map->get_lot( types::point( x, y, 0 ) );
                if ( nullptr != lot && lot->is_traversable() ) {
                    auto& rendr = lot->rendr_lot;
                    rendr->highlight();
                    affected_by_selection.push_back( rendr );
                }
            }
        }
    } else {
        LOG0( "The unit has 0 number of movements!" );
    }
}

void Units::select( const types::id_type renderable_id )
{
    LOG0( "Attempting to select RendrID:", renderable_id );
    auto it = rendr_to_unit.find( renderable_id );
    if ( rendr_to_unit.end() != it ) {
        select( it->second );
    } else {
        LOG0( "No mapping to UnitID from the RendrID" );
    }
}

void Units::unselect()
{
    if ( false == affected_by_selection.empty() ) {
        LOG0( "Unselecting, number of affected terrains: ",
              affected_by_selection.size() );
    }
    for ( auto&& elem : affected_by_selection ) {
        elem->dehighlight();
    }
    affected_by_selection.clear();
}

} //core_units
