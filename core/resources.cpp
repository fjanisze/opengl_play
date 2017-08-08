#include "resources.hpp"

namespace resources {

const Model_resource_def resource_poldek_def = {
    id_unit_poldek,
    resource_type::movable_unit,
    "Poldek",

    "../models/SimpleCar/SimpleCar.obj", //Low res model
    "../models/SimpleCar/SimpleCar.obj", //High res model
};

}
