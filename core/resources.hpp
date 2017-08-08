#ifndef RESOURCES_HPP
#define RESOURCES_HPP
#include <types.hpp>
#include <string>

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

extern const Model_resource_def resource_poldek_def;

}

#endif //RESOURCES_HPP
