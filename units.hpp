#ifndef UNITS_HPP
#define UNITS_HPP

#include <headers.hpp>
#include <types.hpp>
#include <factory.hpp>
#include <models.hpp>
#include <renderable_object.hpp>

namespace game_units {

/*
 * Information about a unit
 * data file: Blender obj file, default
 * color and a pretty name (human readable)
 */
struct Unit_model_data {
    using container = std::vector< Unit_model_data >;
    /*
     * ID given by the configuration, shall uniquely
     * identify a Model_unit_data
     */
    const uint64_t     id;
    /*
     * Path for the obj blender file
     */
    const std::string  model_path;
    /*
     * Color applicable to this unit
     */
    const types::color default_color;
    /*
     * Human readable name
     */
    const std::string  pretty_name;
};

/*
 * Reponsible for loading and manipulating
 * a unit model
 */
class Unit_model
{
public:
    using pointer = std::shared_ptr< Unit_model >;
    using container = std::vector< pointer >;
    explicit Unit_model( const Unit_model_data& data );
    models::my_mesh::meshes& get_meshes();
    operator uint64_t() const
    {
        return model_data.id;
    }
public:
    Unit_model_data model_data;
private:
    models::model_loader::pointer model;
};

namespace internal {
/*
 * TODO: Replace this static list
 * with some config file
 */
extern std::vector<Unit_model_data> units;

}

class Units_container;

/*
 * Object which model a unit, this actually
 * might be either a unit like a car or a
 * construction (building etc)
 */
class Unit : public renderer::Renderable
{
public:
    using pointer = std::shared_ptr< Unit >;
    using container = std::vector< pointer >;
    Unit( Unit_model::pointer unit_model );

    void render( ) override;
private:
    Unit_model::pointer model;
};

/*
 * Wrapper for a real
 * container of units
 */
class Units_container
{
public:
    using pointer = std::shared_ptr< Units_container >;
    using container_type = std::vector< Unit::pointer >;
    Units_container();
    bool add( Unit::pointer unit );
    void remove( Unit::pointer unit );
    std::size_t size() const;
    Unit::pointer find( uint64_t id );
    container_type get();
private:
    id_factory< Units_container > id;
    container_type units;
};

}

#endif //UNITS_HPP
