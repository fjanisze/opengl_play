#ifndef UNITS_MANAGER_HPP
#define UNITS_MANAGER_HPP

#include <headers.hpp>
#include <units.hpp>
#include <terrains.hpp>

namespace graphic_units {

using namespace graphic_terrains;

/*
 * Here we have some utility functions
 * mostly needed for "animation" purpose
 */
class Movements
{
public:
    GLfloat calculate_heading( const types::point& source,
                               const types::point& target ) const;
    /* The unit will point in the direction of the
    * given terrain
    */
    void change_unit_heading( Unit::pointer& unit,
                              const Terrain_lot::pointer& target );
    /*
     * Update the model matrix for the unit in order
     * to make sure that it is placed on the given
     * lot
     */
    void place_unit_on_lot( Unit::pointer& unit,
                            const graphic_terrains::Terrain_lot::pointer& lot );
    /*
     * Usefull function that kind of emulate the acceleration
     * and deceleration of a moving unit, when you start the movement
     * you accelerate till a certain speed, then the unit slow down
     * till it stops
     */
    constexpr GLfloat speed_function( const GLfloat x )
    {
        return ( 2.0f * x - x * x ) * std::pow( sin( x * glm::half_pi<GLfloat>() ), 2.0f );
    }

};

/*
 * Define all the data for a given movement
 */
class Move_processor
{

public:
    Move_processor( Unit::pointer unit,
                    graphic_terrains::Terrain_lot::pointer lot );
    ~Move_processor();
    using pointer = std::shared_ptr< Move_processor >;
    enum struct moving_status {
        READY, //But not started
        MOVING,
        COMPLETED
    };
    moving_status get_status() const;
    void start();
    moving_status step( const types::timestamp& current );
    types::id_type moving_unit_id() const
    {
        return unit->id;
    }
private:
    id_factory< Move_processor > id;
    Unit::pointer unit;
    Terrain_lot::pointer target_lot;
    GLfloat       distance;
    glm::vec3     direction;
    moving_status status;
    types::timestamp start_time;
    types::timestamp arrival_time;
    Movements     movement_impl;
    types::point  original_position;
    GLfloat       heading;
};

/*
 * Information about a given unit,
 * like the terrain lot where it is &c.
 */
struct Unit_info {
    using pointer = std::shared_ptr< Unit_info >;
    Unit::pointer unit;
    graphic_terrains::Terrain_lot::pointer location;
    /*
     * The target location is the place where
     * the unit is moving towards
     */
    Move_processor::pointer movement;
    Unit_info() = default;
};
/*
 * Handle the Unit_info
 */
class Units_data_container
{
public:
    Units_data_container();
    Unit_info::pointer add( const Unit::pointer unit );
    Unit_info::pointer find( const types::id_type id );
    /*
     * The nofail version make the assumption that the unit
     * must be in there somewhere, if not, then panic.
     */
    Unit_info::pointer find_nofail( const types::id_type id );
    std::size_t size() const;
private:
    const id_factory< Units_data_container > id;
    std::unordered_map< types::id_type, Unit_info::pointer > data;
};

/*
 * Object responsible for the processing of unit movements
 */
class Units_movement_processor
{
    /*
     * Manage the unit movement, estimate travel time,
     * update position etc.
     */
    void start_movement( Unit_info::pointer unit );
public:
    Units_movement_processor( Units_data_container& container );
    /*
     * Teleport will just instantly move the unit
     * from one place to another
     */
    bool teleport( types::id_type unit,
                   Terrain_lot::pointer target_lot );
    bool multiple_teleport( const std::vector< types::id_type>& units,
                            graphic_terrains::Terrain_lot::pointer& target_lot );
    /*
     * This is initiate the movement 'animation' of the unit
     * to the target location
     */
    bool move( types::id_type unit_id,
               Terrain_lot::pointer target_lot );
    bool multiple_move( const std::vector< types::id_type >& units,
                        graphic_terrains::Terrain_lot::pointer& target_lot );
    /*
     * Called once for frame (at least), update
     * the units position, manage the movement etc
     */
    void process_movements();
private:
    Movements mov_impl;
    Units_data_container& units_container;
    std::vector< Move_processor::pointer > in_movement;
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
                     graphic_terrains::Terrain_lot::pointer lot );
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
    Units_data_container          units_container;
    Units_movement_processor      movement_processor;
    renderer::Core_renderer_proxy renderer;
};

}

#endif //UNITS_MANAGER_HPP
