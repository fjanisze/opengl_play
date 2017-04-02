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

entity_id map_entity_data::add_at_location(const glm::vec2 &position)
{
    LOG3("Adding entity ",pretty_name," at position: ", position);
    long idx = get_position_idx( position );
    entity_id new_id = new_entity_id();
    auto it = entities.find( idx );
    if( it == entities.end() ) {
        LOG1("Creating a new map_entity, there were none at this position");
        map_entity new_entity;
        new_entity.position = position;
        new_entity.entities.push_back( new_id );
        entities.insert( std::make_pair( idx, std::move( new_entity ) ) );
    } else {
        LOG1("Adding new entity, current amount: ",
             it->second.entities.size() );
        it->second.entities.push_back( new_id );
    }
    return new_id;
}

entities_data_container &map_entity_data::get_data()
{
    return entities;
}

models::model_loader_ptr &map_entity_data::get_model()
{
    return model_ptr;
}

glm::vec3 &map_entity_data::get_color()
{
    return default_color;
}

entity_id map_entity_data::new_entity_id()
{
    static entity_id next_entity_id{ 1 };
    return next_entity_id++;
}

long map_entity_data::get_position_idx(const glm::vec2 &pos) const
{
    /*
     * Using the Cantor Pairing Function
     */
    return ( 1.0f / 2.0f ) * ( pos.x + pos. y) * ( pos.x + pos.y + 1 ) + pos.y;
}


entities_collection::entities_collection(shaders::my_small_shaders *game_shader,
                                         entity_matrix_func lot_pos_generator) :
    shader{ game_shader },
    get_entity_model_matrix{ lot_pos_generator },
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
    LOG3("Entity ",pretty_name," loaded, assigned ID: ", new_model_id);
    add_renderable( this );
    return new_model_id;
}

entity_id entities_collection::add_entity(model_id id,
                                          const glm::vec2 &position)
{
    auto it = entities.find( id );
    if( it == entities.end() ) {
        ERR("Unable to find the entity with ID ", id);
        return invalid_id;
    }
    return it->second->add_at_location( position );
}

void entities_collection::prepare_for_render()
{
    shader->use_shaders();
    GLint view_loc = glGetUniformLocation(*shader,"view");
    GLint projection_loc = glGetUniformLocation(*shader,"projection");

    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    calculate_lighting();
}

void entities_collection::render()
{
    GLint model_loc = glGetUniformLocation(*shader,"model");
    glm::vec3 last_color;
    for( auto& entry : entities )
    {
        entities_data_container& data = entry.second->get_data();
        auto&& meshes = entry.second->get_model()->get_mesh();
        if( last_color != entry.second->get_color() ) {
            last_color = entry.second->get_color();
            apply_object_color( last_color );
        }
        for( auto& entity : data )
        {
            entity.second.model_matrix = get_entity_model_matrix(
                        entity.second.position );
            /*
             * There might be already something there,
             * get the Z component and update the
             * model matrix accordingly
             */

            glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                               glm::value_ptr( entity.second.model_matrix ));
            //Render all the model meshes
            for( auto&& mesh : meshes ) {
                mesh->render( shader );
            }
        }
    }
}

void entities_collection::clean_after_render()
{
    for( auto&& entry : entities ) {
        map_entity_data_ptr entity = entry.second;

    }
}

std::string entities_collection::renderable_nice_name()
{
    return "entities_collection";
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
