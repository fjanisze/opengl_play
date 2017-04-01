#ifndef MAP_ENTITIEST_HPP
#define MAP_ENTITIEST_HPP

#include <shaders.hpp>
#include <models.hpp>
#include <memory>
#include <vector>
#include <logger/log.hpp>
#include <functional>
#include <renderable_object.hpp>
#include <lights.hpp>

namespace map_entities
{

using model_id = long;
using unit_id = long;
constexpr long invalid_id{ -1 };

/*
 * Each map_entity might have multiple
 * instances, like many of the same unit
 * might appear on the map
 */
struct map_entity
{
    /*
     * Once the entity model is loaded
     * multiple instance of the entity might
     * be used, each will have its own entity_id.
     * For example the model of one unit might
     * be rendered multiple time on multiple places
     * when there are multiple units of that type
     * in the game.
     */
    unit_id   entity_id;
    glm::vec2 position;
    glm::mat4 model_matrix;

    map_entity() = default;
};

class map_entity_data;
using map_entity_data_ptr = std::shared_ptr< map_entity_data >;

/*
 * Each loaded entity (model) has
 * its own map_entity object
 */
class map_entity_data
{
public:
    map_entity_data() = default;
    map_entity_data(model_id new_id,
                    const std::string& name,
                    models::model_loader_ptr model ,
                    const glm::vec3 &color);
    /*
     * Add one more of those entities on the map,
     * it might be everything: unit, building..
     */
    unit_id add_entity( const glm::vec2& position );

    template< typename...Args >
    static map_entity_data_ptr create(Args&&...args) {
        return std::make_shared< map_entity_data >(
                    std::forward< Args >( args )... );
    }

private:
    /*
     * The model ID is unique for the specific
     * entity model (model_loader)
     */
    model_id    id;
    /*
     * The pretty name is something that can
     * be rendered to the user, like an entity
     * which is a bunker might have a pretty name:
     * "Light bunker" or "Heavy bunker"..
     */
    std::string pretty_name;

    glm::vec3   default_color;
    models::model_loader_ptr model_ptr;

    /*
     * Store all the instance of the model
     */
    std::vector< map_entity > entities;
};

/*
 * This function is used to obtain the
 * proper model matrix for a given position,
 * the function is implemented in 'terrains'
 */
using entity_matrix_func = std::function<glm::mat4( const glm::vec2&) >;

class entities_collection;
using entity_collection_ptr = std::shared_ptr< entities_collection >;

/*
 * Units, constructions &c are just map
 * entities that can be moved and rendered
 * on the map.
 *
 * Each entity model has its own unique ID,
 * each instance of the entity has its unique
 * ID which is: (instance_ID << 16) & model_ID
 * where instance_ID might not be unique
 * between multiple models.
 */
class entities_collection : public renderable::renderable_object,
        public lights::object_lighting
{
public:
    entities_collection( shaders::my_small_shaders* game_shader,
                         entity_matrix_func lot_pos_generator );
    /*
     * Load the provided obj file
     */
    model_id load_entity( const std::string& model_path,
                          const glm::vec3& default_color,
                          const std::string& pretty_name );

    static entity_collection_ptr create( shaders::my_small_shaders* shad,
                                         entity_matrix_func model_mtrc_gen ) {
        LOG3("Creating new entity_collection, shader ptr: ", shad);
        return std::make_shared< entities_collection >( shad, model_mtrc_gen );
    }
private:
    shaders::my_small_shaders* shader;
    entity_matrix_func get_entity_pos;
    std::map< model_id, map_entity_data_ptr > entities;
    model_id new_entity_id();
};

} //map_entities

#endif //UNITS_HPP