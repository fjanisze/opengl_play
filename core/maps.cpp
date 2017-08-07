#include "maps.hpp"

namespace core_maps {

/*
 * Hardcoded definitions of all the available terrains
 */
const Terrain_lot_def terrain_definitions[] = {
    1, terrain_type::grass,
    {
        traversable_lot::yes
    },
    "Plain Grass", "Grass with very little else..",
    "../models/Grass/grass.obj",
    "../models/Grass/grass.obj",

    2, terrain_type::grass,
    {
        traversable_lot::yes
    },
    "Grass with hills", "A lot of grass with some little hills",
    "../models/Grass/grass2.obj",
    "../models/Grass/grass2.obj",

    3, terrain_type::mountain,
    {
        traversable_lot::no
    },
    "Rocky mountain", "A big rock mountain",
    "../models/Mountain/mountain.obj",
    "../models/Mountain/mountain_highres.obj",

    4, terrain_type::forest,
    {
        traversable_lot::yes
    },
    "Forest", "This is not the black forest!",
    "../models/Forest/Forest.obj",
    "../models/Forest/Forest_complex.obj",
};

}
