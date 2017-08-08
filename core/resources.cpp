#include "resources.hpp"

namespace resources {

/*
 * Units
 */
const Model_resource_def unit_poldek_def = {
    id_unit_poldek,
    resource_type::movable_unit,
    "Poldek",

    "../models/SimpleCar/SimpleCar.obj", //Low res model
    "../models/SimpleCar/SimpleCar.obj", //High res model
};

/*
 * Terrains
 */
const Model_resource_def terrain_grass1_def = {
    id_terrain_grass1,
    resource_type::terrain_lot,
    "Grass",

    "../models/Grass/grass.obj", //Low res model
    "../models/Grass/grass.obj", //High res model
};

const Model_resource_def terrain_grass2_def = {
    id_terrain_grass2,
    resource_type::terrain_lot,
    "Grass 2",

    "../models/Grass/grass2.obj", //Low res model
    "../models/Grass/grass2.obj", //High res model
};

const Model_resource_def terrain_forest_def = {
    id_terrain_forest,
    resource_type::terrain_lot,
    "Forest",

    "../models/Forest/Forest.obj", //Low res model
    "../models/Forest/Forest_complex.obj", //High res model
};

const Model_resource_def terrain_mountain_def = {
    id_terrain_mountain,
    resource_type::terrain_lot,
    "Mountain",

    "../models/Mountain/mountain.obj", //Low res model
    "../models/Mountain/mountain_highres.obj", //High res model
};

}
