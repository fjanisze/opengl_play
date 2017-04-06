#ifndef MAP_ENTITIEST_HPP
#define MAP_ENTITIEST_HPP

#include <shaders.hpp>
#include <models.hpp>
#include <memory>
#include <vector>
#include <logger/log.hpp>
#include <functional>
#include <unordered_map>
#include <renderable_object.hpp>
#include <lights.hpp>
#include <framebuffers.hpp>

namespace map_entities
{

using model_id = long;
using entity_id = long;
constexpr long invalid_id{ -1 };

/*
 * Each map_entity might have multiple
 * instances, like many of the same unit
 * might appear on the map
 */
struct map_entity/* : public renderable::renderable_object,
         public lights::object_lighting*/
{
 /*   void prepare_for_render() override;
    void render() override;
    void clean_after_render() override;*/

    /*
     * Each entity has its own unique ID
     */
    entity_id id;
    /*
     * Once the entity model is loaded
     * multiple instance of the entity might
     * be used, each will have its own entity_id.
     * For example the model of one unit might
     * be rendered multiple time on multiple places
     * when there are multiple units of that type
     * in the game.
     */
    glm::vec2 position;
    glm::mat4 model_matrix;
    /*
     * When static coloring is enabled, this is the color
     * which will be used for rendering the model
     */
    glm::vec3 static_color;
    models::model_loader_ptr model;

    map_entity() {}
};

using map_entities = std::vector< map_entity >;

class map_entity_data;
using map_entity_data_ptr = std::shared_ptr< map_entity_data >;

using entities_data_container = std::unordered_map< long, map_entities >;
/*
 * Each loaded entity (model) has
 * its own map_entity object
 */
class map_entity_data
{
public:
    map_entity_data() = default;
    map_entity_data( model_id new_id,
                     const std::string& name,
                     models::model_loader_ptr model ,
                     const glm::vec3 &color,
                     const glm::vec2& origin);
    /*
     * Add one more of those entities on the map,
     * it might be everything: unit, building..
     */
    entity_id add_at_location(const glm::vec2& position , const glm::mat4 &model_matrix,
                               const glm::vec3& static_color );
    entities_data_container& get_data();
    models::model_loader_ptr& get_model();
    glm::vec3& get_color();

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
    model_id id;
    entity_id new_entity_id();
    /*
     * The pretty name is something that can
     * be rendered to the user, like an entity
     * which is a bunker might have a pretty name:
     * "Light bunker" or "Heavy bunker"..
     */
    std::string pretty_name;

    glm::vec2   coord_origin;
    glm::vec3   default_color;
    models::model_loader_ptr model_ptr;

    /*
     * Store all the instance of the model
     */
    entities_data_container entities;
    /*
     * This is a pairing function from
     * pos.x/pos.y -> n
     */
    long get_position_idx( const glm::vec2& pos ) const;
};

/*
 * Little utility object for managing
 * colors, used for static coloring purpose
 */
class colors_creator
{
public:
    colors_creator( GLfloat step = 1.0f / 100.0f );

    glm::vec3 get_color();
private:
    glm::vec3 next_color;
    GLfloat color_step;
    long num_of_colors;
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
class entities_collection
{
public:
    entities_collection( Framebuffers::framebuffers_ptr frameb,
                         entity_matrix_func lot_pos_generator );
    /*
     * Load the provided obj file
     */
    model_id load_entity( const std::string& model_path,
                          const glm::vec3& default_color,
                          const std::string& pretty_name );
    void set_coord_origin( const glm::vec2& origin );
    /*
     * Return true if there is something under
     * the position x,y
     */
    bool mouse_hover( const GLfloat x,
                      const GLfloat y );
    /*
     * Add an instance of the model
     * on the map
     */
    entity_id add_entity( model_id id, const glm::vec2& position );

    static entity_collection_ptr create( Framebuffers::framebuffers_ptr frameb,
                                         entity_matrix_func model_mtrc_gen ) {
        LOG3("Creating new entity_collection");
        return std::make_shared< entities_collection >( frameb,model_mtrc_gen );
    }
    ~entities_collection() {}
private:
    Framebuffers::framebuffers_ptr framebuffers;
    GLuint entities_backbuffer;
    /*
     * Return the pixel at the position x,y
     * in the framebuffer used to manage
     * mouse movements.
     */
    glm::vec3 read_static_color( const GLfloat x,
                                const GLfloat y );
    /*
     * Map a color (normally the static color)
     * with an index used to uniquely idenfity
     * an entity_id
     */
    long color_to_idx( const glm::vec3& color ) const;
    /*
     * Maps color indexes with entity_ids
     */
    std::unordered_map< long, entity_id > color_entity_mapping;
    entity_id entity_under_focus;
    entity_matrix_func get_entity_model_matrix;
    std::unordered_map< model_id, map_entity_data_ptr > entities;
    model_id new_entity_id();
    glm::vec2 coord_origin;
    colors_creator colors;
    bool static_coloring;
    /*
     * Rendering functions
     */
 /*   void prepare_for_render() override;
    void render() override;
    void clean_after_render() override;
    std::string renderable_nice_name() override;*/
};

} //map_entities

#endif //UNITS_HPP
