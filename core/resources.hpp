#ifndef RESOURCES_HPP
#define RESOURCES_HPP
#include <types.hpp>
#include <string>
#include <memory>

namespace resources {

/*
 * Type of resource
 */
enum class resource_type {
    terrain_lot,
    movable_unit,
};

/*
 * List of unique IDs assigned to units
 */
constexpr types::id_type id_unit_poldek{ 1 };

/*
 * List of unique IDs assigned to terrains
 */
constexpr types::id_type id_terrain_grass1{ 2 };
constexpr types::id_type id_terrain_grass2{ 3 };
constexpr types::id_type id_terrain_forest{ 4 };
constexpr types::id_type id_terrain_mountain{ 5 };

/*
 * Model definition struct, this is in common between
 * multiple parts of the game, in particular between the
 * core component and the ui/rendering part.
 */
struct Model_resource_def {
    const types::id_type id;
    const resource_type  type;
    const std::string    pretty_name;

    const std::string    model_path;
    const std::string    high_res_model_path;
};

/*
 * All the definitions
 */
extern const Model_resource_def unit_poldek_def;

extern const Model_resource_def terrain_grass1_def;
extern const Model_resource_def terrain_grass2_def;
extern const Model_resource_def terrain_forest_def;
extern const Model_resource_def terrain_mountain_def;

}

namespace core_units {

/*
 * One entry for each possible type of unit
 */
enum class unit_type {
    small_vehicle,
};

/*
 * Some specification details about the units
 */
struct Unit_specs {
    int num_of_movements;

    bool has_avail_movements() const
    {
        return 0 < num_of_movements;
    }
};

/*
 * Definition of a unit, there should be a definition for
 * each available unit.
 */
struct Unit_def {
    const resources::Model_resource_def& model_def;
    const std::string    name;

    const unit_type      type;
    const Unit_specs     specs;

    const std::string    desc;
};

struct Unit_config {
    using pointer = std::shared_ptr< Unit_config >;
    const Unit_def def;

    Unit_specs current_specs;

    Unit_config( const Unit_def& definition ) :
        def{ definition },
        current_specs{ definition.specs }
    {}
};

}

namespace core_maps {

enum class terrain_type {
    grass,
    mountain,
    forest,
};

enum class traversable_lot {
    yes,
    no
};

/*
 * Characteristics of a terrain lot, if units can move over it,
 * if there's a penalty for the move etc.
 */
struct Lot_spec {
    traversable_lot traversable;
};

/*
 * Definition of a terrain lot, include all the information
 * needed to draw the lot
 */
struct Terrain_lot_def {
    const resources::Model_resource_def& model_def;
    const std::string    name;

    const terrain_type   type;
    const Lot_spec specs;

    const std::string    desc;
};

struct Lot_config {
    using pointer = std::shared_ptr< Lot_config >;
    //Those are the default values for the lot
    const Terrain_lot_def def;
    //Those are the current values
    Lot_spec current_specs;

    Lot_config( const Terrain_lot_def definition ) :
        def{ definition },
        current_specs{ definition.specs }
    {}
};

}

#endif //RESOURCES_HPP
