#include "units.hpp"
#include <logger/logger.hpp>
#include "resources.hpp"
#include <stack>

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
              core_maps::Map::pointer map )
{
    LOG3( "Created!" );
    data.game_map = map;
    data.rendr_units = units;
    for ( const auto& elem : units_definitions ) {
        LOG0( "Unit ID:", elem.model_def.id, ", model name:", elem.model_def.pretty_name,
              ", model paths:", elem.model_def.model_path, ",",
              elem.model_def.high_res_model_path );
    }
    selection = factory< Unit_selection >::create( data );
}

Unit::pointer Units::create( const types::id_type id )
{
    LOG3( "Attempting to create unit with modelID:", id );
    auto& unit_def = find_unit_definition( id );

    auto new_unit = factory< Unit >::create( unit_def );
    new_unit->rendr_unit = data.rendr_units->create_unit( id );
    if ( nullptr == new_unit->rendr_unit ) {
        ERR( "Creation failed!" );
        return nullptr;
    }
    units[ new_unit->id ] = new_unit;
    data.rendr_to_unit[ new_unit->rendr_unit->id ] = new_unit;
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
    return data.rendr_units->place_unit( unit->rendr_unit, lot->rendr_lot );
}

void Unit_selection::select( Unit::pointer& unit )
{
    if ( unit->config->current_specs.has_avail_movements() ) {
        LOG0( "Attempt to selected UnitID:", unit->id );
        /*
         * Show the reachable area for this unit,
         * highlight the accessible lots
         */
        const int mov_cnt{ unit->config->current_specs.num_of_movements };
        const float sx{ unit->position.x };
        const float sy{ unit->position.y };
        for ( float x{ sx - mov_cnt } ; x <= sx + mov_cnt ; ++x ) {
            for ( float y{ sy - mov_cnt } ; y <= sy + mov_cnt ; ++y ) {
                auto lot = data.game_map->get_lot( types::point( x, y, 0 ) );
                if ( nullptr != lot && lot->is_traversable() ) {
                    auto& rendr = lot->rendr_lot;
                    rendr->highlight();
                    affected_by_selection.push_back( rendr );
                }
            }
        }
        selected_unit = unit;
    } else {
        LOG0( "The unit has 0 number of movements!" );
    }
}

void Unit_selection::select( const types::id_type renderable_id )
{
    auto it = data.rendr_to_unit.find( renderable_id );
    if ( data.rendr_to_unit.end() != it ) {
        if ( selected_unit == nullptr ||
                it->second->id != selected_unit->id ) {
            select( it->second );
        }
    } else {
        LOG0( "No mapping to UnitID from the RendrID" );
    }
}

void Unit_selection::unselect()
{
    if ( false == affected_by_selection.empty() ) {
        LOG0( "Unselecting, number of affected terrains: ",
              affected_by_selection.size() );
    }
    for ( auto&& elem : affected_by_selection ) {
        elem->dehighlight();
    }
    for ( auto&& elem : selected_path ) {
        elem->rendr_lot->dehighlight();
    }
    affected_by_selection.clear();
    selected_unit.reset();
}

void Units::highlight_path( core_maps::Map_lot::pointer& target_lot )
{
    if ( nullptr != selection->selected_unit ) {
        if ( target_lot->id != selection->cur_target_lot_id ) {
            LOG0( "Attempt to find a path for the UnitID:", selection->selected_unit->id,
                  ",to the target LotID:", target_lot->id, ", which is traversable: ",
                  target_lot->is_traversable() );
            core_maps::Map_lot::pointer root = data.game_map->get_lot( selection->selected_unit->position );
            if ( nullptr == root ) {
                PANIC( "Not able to find the ROOT lot!" );
            }
            /*
             * Some lots might be highlighted from the former operation,
             * clean those lots (dehighlight)
             */
            if ( false == selection->selected_path.empty() ) {
                for ( auto&& elem : selection->selected_path ) {
                    bool do_not_dehighlight{ false };
                    for ( auto&& selected : selection->affected_by_selection ) {
                        if ( selected->id == elem->rendr_lot->id ) {
                            do_not_dehighlight = true;
                            break;
                        }
                    }
                    if ( false == do_not_dehighlight ) {
                        elem->rendr_lot->dehighlight();
                    }
                }
                selection->selected_path.clear();
            }
            /*
             * Now look for the path to highlight
             */
            if ( target_lot->is_traversable() ) {
                auto path = data.game_map->paths.shortest( root, target_lot );
                if ( false == path.empty() ) {
                    /*
                     * Highlight the path!
                     */
                    selection->selected_path = path;
                    selection->cur_target_lot_id = target_lot->id;
                }
            }
        }
        /*
         * Highlight the path!
         */
        for ( auto&& lot : selection->selected_path ) {
            lot->rendr_lot->highlight();
        }
    }

}

} //core_units
