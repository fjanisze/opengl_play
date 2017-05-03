#include <units.hpp>

namespace game_units
{

namespace internal
{

Unit_model_data::container units= {
    { 1, "../models/SimpleCar/SimpleCar.obj", types::color(1.0f), "Poldek" }
};

}

Unit_model::Unit_model(const Unit_model_data& data ) :
    model_data{ data }
{
    LOG3("Loading model, path: ", data.model_path,
         ", default color: ", data.default_color );
    model = factory< models::model_loader >::create(
                data.model_path
                );
    if( false == model->load_model() ) {
        PANIC("Not able to load the requested model!");
    }
}

models::my_mesh::meshes &Unit_model::get_meshes()
{
    return model->get_mesh();
}

Unit::Unit( Unit_model::pointer unit_model ) :
    model{ unit_model }
{
    LOG3("New unit with ID: ",id,", created! Pretty name: ",
         unit_model->model_data.pretty_name );
}


bool Units_container::place_unit( Unit::pointer unit )
{
    LOG3( "Placing a new unit, ID:",unit->id );
    if( nullptr != find_unit( unit->id ) ) {
        WARN2("A unit of the same ID is already here!");
        return false;
    }
    units.push_back( unit );
    return true;
}

void Units_container::remove_unit( Unit::pointer unit )
{
    LOG3("Removing unit with ID:", unit->id);
    units.erase( std::remove_if(
                     units.begin(),
                     units.end(),
                     [ &unit ]( const Unit::pointer& ptr ) {
                         return ptr->id == unit->id;
                     }),
                 units.end());
}

std::size_t Units_container::size() const
{
    return units.size();
}

Unit::pointer Units_container::find_unit(uint64_t id)
{
    for( auto&& unit : units ) {
        if( unit->id == id ) {
            return unit;
        }
    }
}

Units_container::container Units_container::get()
{
    return units;
}


}
