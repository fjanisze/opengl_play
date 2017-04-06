#include "map_entities.hpp"

namespace map_entities
{

/*
void map_entity::prepare_for_render()
{

}

void map_entity::render()
{

}

void map_entity::clean_after_render()
{

}

*/

map_entity_data::map_entity_data(model_id new_id,
                                 const std::string &name,
                                 models::model_loader_ptr model,
                                 const glm::vec3& color,
                                 const glm::vec2& origin) :
    pretty_name{ name },
    id{ new_id },
    default_color{ color },
    model_ptr{ model },
    coord_origin{ origin }
{
    LOG3("New entity: ",name,", model ID: ",id);
}

entity_id map_entity_data::add_at_location(const glm::vec2 &position,
                                           const glm::mat4& model_matrix,
                                           const glm::vec3 &static_color)
{
    LOG3("Adding entity ",pretty_name," at position: ", position);
    long idx = get_position_idx( position );
    //New Entity
    entity_id new_id = new_entity_id();
    map_entity new_entity;
    new_entity.position = position;
    new_entity.static_color = static_color;
    new_entity.id = new_id;
    new_entity.model_matrix = model_matrix;
    new_entity.model = model_ptr;
    //Add the entity
    entities[ idx ].emplace_back( std::move( new_entity ) );
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
    GLfloat x = pos.x + coord_origin.x + 5;
    GLfloat y = pos.y + coord_origin.y + 5;
    return ( 1.0f / 2.0f ) * ( x + y ) * ( x + y + 1 ) + y;
}


entities_collection::entities_collection(Framebuffers::framebuffers_ptr frameb,
                                         entity_matrix_func lot_pos_generator) :
    framebuffers{ frameb },
    get_entity_model_matrix{ lot_pos_generator },
    coord_origin{ glm::vec2(0.0f) },
    static_coloring{ false },
    entity_under_focus{ invalid_id }
{
    LOG3("Creating new entity collection");
    entities_backbuffer = framebuffers->create_buffer();
    if( entities_backbuffer > 0 ) {
        LOG3("Enabling static coloring with back framebuffer with ID: ",
             entities_backbuffer);
        static_coloring = true;
    }
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
                                    default_color,
                                    coord_origin);

    entities.insert( std::make_pair( new_model_id, new_entity ) );
    LOG3("Entity ",pretty_name," loaded, assigned ID: ", new_model_id);
    return new_model_id;
}

void entities_collection::set_coord_origin(const glm::vec2 &origin)
{
    LOG3("Setting coordinate origin at ", origin);
    coord_origin = origin;
}

bool entities_collection::mouse_hover(const GLfloat x,
                                      const GLfloat y)
{
    if( true == static_coloring ) {
        glm::vec3 color = read_static_color( x, y );
        if( color != glm::vec3(0.0f) ) {
            long color_idx = color_to_idx( color );
            auto it = color_entity_mapping.find( color_idx );
            if( it == color_entity_mapping.end() )
            {
                ERR("A color has been discovered, but no entity was found!: ",
                    color,", ", color_idx);
            } else if( entity_under_focus != it->second ) {
                LOG1("The mouse is over the entity: ",
                     it->second);
                entity_under_focus = it->second;
            }
            return true;
        } else {
            entity_under_focus = invalid_id;
        }
    }
    return false;
}


entity_id entities_collection::add_entity(model_id id,
                                          const glm::vec2 &position)
{
    auto it = entities.find( id );
    if( it == entities.end() ) {
        ERR("Unable to find the entity with ID ", id);
        return invalid_id;
    }

    glm::vec3 static_color = colors.get_color();
    glm::vec3 static_color_int;
    for( int i {0}; i < 3; ++i ) {
        static_color_int[ i ] = std::floor( static_color[ i ] * 255.0f + 0.5f );
    }
    long static_color_idx = color_to_idx( static_color_int );
    entity_id new_id = it->second->add_at_location( position,
                                        get_entity_model_matrix( position ),
                                        static_color );
    LOG3("Entity ID: ", new_id,", with static color: ",
         static_color_int,", mapped to IDX: ",
         static_color_idx);
    color_entity_mapping[ static_color_idx ] = new_id;
    return new_id;
}

glm::vec3 entities_collection::read_static_color(const GLfloat x,
                                                const GLfloat y)
{
    framebuffers->bind( entities_backbuffer );
    GLubyte pixels[3] = { 0,0,0 };
    glReadPixels( x, y,
                  1, 1,
                  GL_RGB,
                  GL_UNSIGNED_BYTE,
                  &pixels);
    framebuffers->unbind();
    return glm::vec3( pixels[0], pixels[2], pixels[1] );
}

long entities_collection::color_to_idx(const glm::vec3 &color) const
{
    return (long) ( color.r ) << 16 |
           (long) ( color.b ) << 8 |
           (long) ( color.g );
}
/*
void entities_collection::prepare_for_render()
{
    shader->use_shaders();
    GLint view_loc = glGetUniformLocation(*shader,"view");
    GLint projection_loc = glGetUniformLocation(*shader,"projection");

    glUniformMatrix4fv(view_loc, 1,
                       GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(projection_loc, 1,
                       GL_FALSE, glm::value_ptr(projection_matrix));

    if( true == static_coloring )
    {
        framebuffers->bind( entities_backbuffer );
        glClearColor(0.0,0.0,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        framebuffers->unbind();
    }
    calculate_lighting();
}

void entities_collection::render()
{
    GLint model_loc = glGetUniformLocation(*shader,"model");
    glm::vec3 last_color;
    for( auto& entity : entities )
    {
        entities_data_container& data = entity.second->get_data();
        /*
         * The set of meshes is the same for all
         * the instances of this model
         */
  /*      auto&& meshes = entity.second->get_model()->get_mesh();
        if( last_color != entity.second->get_color() )
        {
            /*
             * Each model has just ONE default color,
             * but each instance of the model has a
             * different static color. Those colors
             * do not have the same meaning..
             */
       /*     last_color = entity.second->get_color();
            apply_object_color( last_color );
        }
        for( auto& entity_instance : data )
        {
            for( auto& entity_location : entity_instance.second )
            {
                glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                                   glm::value_ptr( entity_location.model_matrix ));
                //Render all the model meshes
                for( auto&& mesh : meshes ) {
                    mesh->render( shader );
                }
                /*
                 * Render again on the back framebuffer
                 * using the static color
                 */
      /*          if( true == static_coloring )
                {
                    framebuffers->bind( entities_backbuffer );
                    shader->force_single_color( entity_location.static_color );
                    for( auto&& mesh : meshes ) {
                        mesh->render( shader );
                    }
                    shader->force_single_color();
                    framebuffers->unbind();
                }
            }
        }
    }
}

void entities_collection::clean_after_render()
{
}

std::string entities_collection::renderable_nice_name()
{
    return "entities_collection";
}
*/
/*
 * Return a new unique model_id
 */
model_id entities_collection::new_entity_id()
{
    static model_id next_model_id{ 1 };
    return next_model_id++;
}

colors_creator::colors_creator( GLfloat step ) :
    color_step{ step },
    next_color{ glm::vec3( 1.0f ) },
    num_of_colors{ 0 }
{
    if( step <= 0.0f ) {
        ERR("Negative step not allowed!");
        throw std::runtime_error("Using negative step value in colors_creator");
    }
    LOG3("Number of supported colors: ",
         std::pow( glm::floor( 1.0f / step) , 3 ));
}

glm::vec3 colors_creator::get_color()
{
    glm::vec3 color = next_color;
    auto apply_step = [this]( GLfloat& component ) {
        component -= color_step;
        if( component < 0 ) {
            component = 1.0f;
        }
        return component;
    };

    /*
     * Calculate the next color, note that
     * if we complete all the available colors
     * give the color_step, then the generation
     * restart from the beginning (white color)
     */
    for( int i{ 2 } ; i >= 0 ; ++i ) {
        if( apply_step( next_color[i] ) != 1.0f ) {
            break;
        }
    }
    ++num_of_colors;
    return color;
}


}
