#include <headers.hpp>
#include <types.hpp>
#include <factory.hpp>
#include <models.hpp>

#ifndef UNITS_HPP
#define UNITS_HPP

namespace game_units
{

/*
 * Reponsible for loading and manipulating
 * a unit model
 */
class Unit_models
{
public:
    using pointer = std::shared_ptr< Unit_models >;
    explicit Unit_models( const std::string& model_path,
                 const types::color& default_color );
    types::color get_color();
    models::my_mesh::meshes& get_meshes();
private:
    models::model_loader::pointer model;
    types::color color;
};

/*
 * Information about a unit
 * data file: Blender obj file, default
 * color and a pretty name (human readable)
 */
struct Unit_data
{
    const std::string model_path;
    const types::color default_color;
    const std::string pretty_name;
};

namespace internal
{
/*
 * TODO: Replace this static list
 * with some config file
 */
extern std::vector<Unit_data> units;

}

/*
 * Object which model a unit, this actually
 * might be either a unit like a car or a
 * construction (building etc)
 */
class Unit
{
public:
    using pointer = std::shared_ptr< Unit >;
    using container = std::vector< pointer >;
    Unit( const Unit_data unit_data,
          Unit_models::pointer unit_model );
public:
    const id_factory< Unit > id;
    const Unit_data data;
private:
    Unit_models::pointer model;
};

/*
 * Object which handle the creation, destruction
 * of units. Units placement, movement etc..
 */
class Units
{
public:
    using pointer = std::shared_ptr< Units >;
    Units();
    uint64_t get_unit_id( const std::string& name );
private:
    Unit::container available_units;
};

}

#endif //UNITS_HPP
