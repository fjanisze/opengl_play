#include "map_entities.hpp"

namespace map_entities
{

map_entity_data::map_entity_data(model_id new_id,
                                 const std::string &name,
                                 models::model_loader_ptr model,
                                 const glm::vec3& color ) :
    pretty_name{ name },
    id{ new_id },
    default_color{ color },
    model_ptr{ model }
{
    LOG3("New entity: ",name,", model ID: ",id);
}

unit_id map_entity_data::add_entity(const glm::vec2 &position)
{
    LOG3("Adding entity ",pretty_name," at position: ", position);
}


entities_collection::entities_collection(shaders::my_small_shaders *game_shader,
                                         entity_matrix_func lot_pos_generator) :
    shader{ game_shader },
    get_entity_pos{ lot_pos_generator },
    object_lighting( game_shader )
{
    LOG3("Creating new entity collection");
}

model_id entities_collection::load_entity(const std::string &model_path,
                                          const glm::vec3 &default_color,
                                          const std::string &pretty_name)
{
    LOG3("Loading model: ", model_path,", name: ", pretty_name);

    models::model_loader_ptr new_model = models::model_loader::load(
                model_path );

    if( nullptr == new_model ) {
        ERR("Model loading failed!");
        return invalid_id;
    }

    model_id new_model_id = new_entity_id();
    map_entity_data_ptr new_entity = map_entity_data::create( new_model_id,
                                    pretty_name,
                                    new_model,
                                    default_color );

    entities.insert( std::make_pair( new_model_id, new_entity ) );
    LOG3("Entity loaded, assigned ID: ", new_model_id);
}

/*
 * Return a new unique model_id
 */
model_id entities_collection::new_entity_id()
{
    static model_id next_model_id{ 1 };
    return next_model_id++;
}

}
