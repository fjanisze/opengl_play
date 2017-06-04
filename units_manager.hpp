#include <headers.hpp>
#include <units.hpp>
#include <terrains.hpp>

namespace game_units
{

/*
 * Information about a given unit,
 * like the terrain lot where it is &c.
 */
struct Unit_info
{
    using pointer = std::shared_ptr< Unit_info >;
    Unit::pointer target_unit;
    game_terrains::Terrain_lot::pointer location_lot;
};
/*
 * Handle the Unit_info
 */
class Unit_info_container
{
public:
    Unit_info::pointer add( const Unit::pointer unit );
    Unit_info::pointer find( const types::id_type id );
    std::size_t size() const;
private:
    std::unordered_map< types::id_type, Unit_info::pointer > data;
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
    /*
     * Move the unit with ID to the provided
     * target lot
     */
    bool move_unit( const types::id_type unit_id,
                    game_terrains::Terrain_lot::pointer target_lot );
private:
    /*
     * Those are the units we ca use for
     * building, each call to create_unit
     * must be for one of the available units
     */
    Unit_model::container available_models;
    Unit_model::pointer find_model( const uint64_t id );
    /*
     * Update the model matrix for the unit in order
     * to make sure that it is placed on the given
     * lot
     */
    void update_unit_position( Unit::pointer& unit,
                               const game_terrains::Terrain_lot::pointer& lot );
    /*
     * Those are the units which were actually
     * create by the user (possibly placed somewhere).
     */
    Unit_info_container units_container;
    renderer::Core_renderer_proxy renderer;
};

}
