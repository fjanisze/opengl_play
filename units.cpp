#include <units.hpp>

namespace game_units
{

namespace internal
{

std::vector<Unit_data> units= {
    { "../models/SimpleCar/SimpleCar.obj", types::color(1.0f), "Poldek" }
};

}

Unit_models::Unit_models(const std::string &model_path,
                         const types::color &default_color) :
    color{ default_color }
{
    LOG3("Loading model, path: ", model_path,
         ", default color: ", default_color );
    model = factory< models::model_loader >::create(
                model_path
                );
    if( false == model->load_model() ) {
        PANIC("Not able to load the requested model!");
    }
}

types::color Unit_models::get_color()
{
    return color;
}

models::my_mesh::meshes &Unit_models::get_meshes()
{
    return model->get_mesh();
}


Unit::Unit( const Unit_data unit_data,
           Unit_models::pointer unit_model) :
    data{ unit_data },
    model{ unit_model }
{
    LOG3("New unit with ID: ",id,", created! Pretty name: ",
         data.pretty_name );
}

Units::Units()
{

    LOG3("Loading ", internal::units.size()," units!");
    for( auto&& unit : internal::units )
    {
        Unit_models::pointer model = factory< Unit_models >::create(
                    unit.model_path,
                    unit.default_color );
        Unit::pointer new_unit = factory< Unit >::create(
                    unit,
                    model );
        available_units.push_back( new_unit );
    }
    LOG3("Completed! Amount of available units: ",
         available_units.size() );
}

uint64_t Units::get_unit_id(const std::string &name)
{
    for( auto&& unit : available_units ) {
        if( unit->data.pretty_name == name ) {
            return unit->id;
        }
    }
    return constants::INVALID_ID;
}

}
