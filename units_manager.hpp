#include <headers.hpp>
#include <units.hpp>
#include <terrains.hpp>

namespace game_units
{

using namespace game_terrains;

/*
 * Information about a given unit,
 * like the terrain lot where it is &c.
 */
struct Unit_info
{
    using pointer = std::shared_ptr< Unit_info >;
    Unit::pointer target_unit;
    game_terrains::Terrain_lot::pointer location_lot;
    Unit_info() = default;
};
/*
 * Handle the Unit_info
 */
class Units_movement_data
{
public:
    Unit_info::pointer add( const Unit::pointer unit );
    Unit_info::pointer find( const types::id_type id );
    std::size_t size() const;
private:
    std::unordered_map< types::id_type, Unit_info::pointer > data;
};

/*
 * Process the 'animation' of a moving unit
 */
class Units_movement_animator
{

};

/*
 * Object responsible for the processing of unit movements
 */
class Units_movement_processor
{
public:
    Units_movement_processor( Units_movement_data& container );
    /*
     * Teleport will just instantly move the unit
     * from one place to another
     */
    bool teleport( types::id_type unit,
                   Terrain_lot::pointer target_lot );
    bool multiple_teleport( const std::vector< types::id_type>& units,
                             game_terrains::Terrain_lot::pointer &target_lot );
private:
    /*
     * Update the model matrix for the unit in order
     * to make sure that it is placed on the given
     * lot
     */
    void update_unit_position( Unit::pointer& unit,
                               const game_terrains::Terrain_lot::pointer& lot );
    Units_movement_data& units_container;
};

/*
 * Object which handle the creation, destruction
 * of units. Units placement, movement etc..
 */
class Units
{
public:
    using pointer = std::shared_ptr< Units >;
    explicit Units( renderer::Core_renderer_proxy renderer );
    /*
     * Return the set of units that Units is able
     * to build or recognize
     */
    Unit_model_data::container buildable_units();

    /*
     * Return a new instance of a buildable
     * unit, the provided id must be for one
     * of the available model
     */
    Unit::pointer create_unit( uint64_t id );

    /*
     * Place a unit on a given terrain lot
     */
    bool place_unit( Unit::pointer unit,
                     game_terrains::Terrain_lot::pointer lot );
    Units_movement_processor& movements();
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
    Units_movement_data           units_container;
    Units_movement_processor      movement_processor;
    renderer::Core_renderer_proxy renderer;
};

}
