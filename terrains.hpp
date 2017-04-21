#ifndef TERRAINS_HPP
#define TERRAINS_HPP

#include <shaders.hpp>
#include <models.hpp>
#include <unordered_map>
#include <initializer_list>
#include <set>
#include <renderable_object.hpp>
#include <lights.hpp>
#include <types.hpp>
#include <renderable_object.hpp>

namespace terrains
{

/*
 * For each lot (ID) here we store up to two
 * models, one low res (normal res) one high res).
 * For closeup scenes 'terrains' will load the high
 * res model
 */
struct lot_models
{
    glm::vec4 default_color;
    /*
     * If not provided, the high_res_model is the
     * same as the low res..
     */
    models::model_loader_ptr low_res_model;
    models::model_loader_ptr high_res_model;
};

class terrains;
using terrains_ptr = std::shared_ptr<terrains>;

template<typename T>
using vec_of_vecs = std::vector< std::vector< T > >;
/*
 * This is the type of the matrix which
 * define a 2D map in terms of terrain ID's
 */
using terrain_map_t = vec_of_vecs< long >;

/*
 * Internal reppresentation of a terrain lot
 */
struct terrain_lot : public renderable::renderable_object
{
    long terrain_id; //Must match one of the unique loaded terrains
    lot_models model;
    glm::vec2 position;
    GLfloat   height;
    /*
     * The ID assigned by the core renderer
     */
    renderable::renderable_id rendr_id;
    bool visible; //True when this lot should be rendered

    terrain_lot() = default;
    bool operator < ( const terrain_lot& other ) {
        if( position.x < other.position.x ) {
            return true;
        } else if( position.x == other.position.x ) {
            return position.y < other.position.y;
        }
        return false;
    }

    /*
     * Rendering functions
     */
    void prepare_for_render( shaders::shader_ptr& shader ) override;
    void render( shaders::shader_ptr& shader ) override;
    void clean_after_render( shaders::shader_ptr& shader ) override;
};

using lot_pointer = std::shared_ptr< terrain_lot >;

/*
 * 'terrains' support two model quality
 */
enum class rendering_quality {
    highres,
    lowres
};

/*
 * Some rendering statistic, mostly used
 * to optimize the terrain rendering procedure.
 */
struct rendr_eng_data
{
    /*
     * this value define the maximum/min amount of
     * meshes that could be rendered in order
     * to allow the use of highres/lowres models
     */
    const long highres_shift_max_mesh{ 400 };
    const long lowres_shift_min_mesh{ 1400 };
    rendering_quality current_rendr_quality;

    /*
     * Real number of meshes rendered
     */
    long num_of_rendered_mesh;

    rendr_eng_data() :
        current_rendr_quality{ rendering_quality::lowres }
    { }
};

/*
 * Handle the game map, which is a matrix of lots
 * of differnet kinds.
 */
class terrains
{
public:
    terrains(renderable::renderer_pointer rendr );
    /*
     * Load a terrain model, the user might provide its
     * own identificator. If not, a new unique one will be created
     */
    long load_terrain(const std::string& model_filename,
                      const glm::vec4 &color,
                      long terrain_id = -1);
    /*
     * To load the highres model first the lowres need to be loaded,
     * the code will use the generated terrain_id from load_terrain
     * to upload the highres model
     */
    long load_highres_terrain( const std::string& model_filename,
                               long terrain_id );
    /*
     * The terrain map define how this terrain looks like,
     * the position of the loaded textures. The map should
     * contain the ID's used to load the terrains
     */
    bool load_terrain_map(const terrain_map_t& map,
                          GLfloat lot_size, //Each lot is a square: lot_size X lot_size
                          glm::vec2 central_lot); //Position of the lot at the center (0,0)

    //Factory
    static terrains_ptr create( renderable::renderer_pointer rendr) {
        return std::make_shared<terrains>( rendr);
    }

    void mouse_hover( const types::ray_t& dir );
    /*
     * This is where the camera is pointing, the center of
     * the view field. The idea is to draw only a certain
     * amount of lots which are in the visible area.
     */
    void set_view_center( const glm::vec2& pos,
                          const GLfloat distance );
    /*
     * Return the model matrix for the lot
     * at position pos
     */
    glm::mat4 get_lot_model_matrix( const glm::vec2& pos ) const;
    /*
     * Return the model matrix for the lot
     * with an additional offset accounting
     * for the height.
     */
    glm::mat4 get_lot_top_model_matrix( const glm::vec2& pos ) const;
    /*
     * Return the corresponding lot position
     * for the given ray or (inf,inf) if not
     * existing any lot for that position
     */
    glm::vec2 get_lot_position( const types::ray_t &dir ) const;
    bool is_a_valid_position( const glm::vec2& pos ) const;
    glm::vec2 get_coord_origin() const;
    void unselect_highlighted_lot();
private:
    renderable::renderer_pointer renderer;

    std::unordered_map<long,lot_models> terrain_container;

    GLfloat lot_size;
    GLfloat get_position_idx( const glm::vec2& pos ) const;
    //std::vector<terrain_lot> terrain_map;
    std::unordered_map< long, lot_pointer > terrain_map;
    std::set< long > used_ids;
    glm::vec2 view_center;
    /*
     * The origins lot is the lot from which
     * the coordinate numbering starts
     */
    glm::vec2 origins_lot;
    /*
     * This variable specify how far from the view_center
     * lots should be rendered
     */
    GLfloat   field_of_view_distance;
    void update_rendr_quality( long rendr_mesh_cnt );

    void select_highlighted_lot(const glm::vec2 &lot );
    bool is_highlighted(const glm::vec2 &lot ) const;
    glm::vec3 highlight_lot_color( const glm::vec3& color ) const;
    glm::vec2 highlighted_lot;//With a little brighter color

    rendr_eng_data rendering_data;
    long generate_unique_id();

    /*
     * Rendering functions
     */
  /*  void prepare_for_render() override;
    void render() override;
    void clean_after_render() override;*/
};

}

#endif
