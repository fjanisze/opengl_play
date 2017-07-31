#include <units.hpp>

namespace game_units {

namespace internal {

Unit_model_data::container units = {
    { 1, "../models/SimpleCar/SimpleCar.obj", types::color( 1.0f ), "Poldek" }
};

}

Unit_model::Unit_model( const Unit_model_data& data ) :
    model_data{ data }
{
    LOG3( "Loading model, path: ", data.model_path,
          ", default color: ", data.default_color );
    model = factory< models::model_loader >::create(
                data.model_path
            );
    if ( false == model->load_model() ) {
        PANIC( "Not able to load the requested model!" );
    }
}

models::my_mesh::meshes& Unit_model::get_meshes()
{
    return model->get_mesh();
}

Unit::Unit( Unit_model::pointer unit_model ) :
    model{ unit_model }
{
    LOG3( "New unit with ID: ", id,
          ", created! Pretty name: ",
          unit_model->model_data.pretty_name );
    rendering_data.default_color = unit_model->model_data.default_color;
}

void Unit::render()
{
    for ( auto&& mesh : model->get_meshes() ) {
        mesh->render( rendering_data.shader );
    }
}


Units_container::Units_container()
{
    LOG0( "New container with ID:", id, " Created!" );
}

bool Units_container::add( Unit::pointer unit )
{
    LOG3( "Placing a new unit, ID:", unit->id,
          ", container ID:", id );
    if ( nullptr != find( unit->id ) ) {
        WARN2( "A unit of the same ID is already here!" );
        return false;
    }
    units.push_back( unit );
    LOG1( "Amount of units:", units.size() );
    return true;
}

void Units_container::remove( Unit::pointer unit )
{
    LOG3( "Removing unit with ID:", unit->id,
          ", amount of units ", units.size(),
          ", container ID:", id );
    units.erase( std::remove_if(
                     units.begin(),
                     units.end(),
    [ &unit ]( const Unit::pointer & ptr ) {
        return ptr->id == unit->id;
    } ),
    units.end() );
    LOG1( "Amount of units after removal: ", units.size() );
}

std::size_t Units_container::size() const
{
    return units.size();
}

Unit::pointer Units_container::find( uint64_t id )
{
    for ( auto&& unit : units ) {
        if ( unit->id == id ) {
            return unit;
        }
    }
    return nullptr;
}

Units_container::container_type Units_container::get()
{
    return units;
}


}
