#ifndef CORE_MAPS_HPP
#define CORE_MAPS_HPP
#include <factory.hpp>
#include <types.hpp>
#include "resources.hpp"

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
struct Terrain_lot_spec {
    const traversable_lot traversable;
};

/*
 * Definition of a terrain lot, include all the information
 * needed to draw the lot
 */
struct Terrain_lot_def {
    const resources::Model_resource_def& model_def;
    const std::string    name;

    const terrain_type   type;
    const Terrain_lot_spec specs;

    const std::string    desc;
};

class Map
{
public:
    Map( const std::string& name,
         const size_t size );
};

class Maps
{
public:
    Maps();

};

} //core_maps

#endif //CORE_MAPS_HPP
