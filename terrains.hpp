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

namespace terrains
{

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
struct terrain_lot
{
    long terrain_id; //Must match one of the unique loaded terrains
    glm::vec2 position;
    glm::mat4 model_matrix;
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
};
using lot_map_t = vec_of_vecs< terrain_lot >;

/*
 * For each lot (ID) here we store up to two
 * models, one low res (normal res) one high res).
 * For closeup scenes 'terrains' will load the high
 * res model
 */
struct lot_models
{
    glm::vec3 default_color;
    /*
     * If not provided, the high_res_model is the
     * same as the low res..
     */
    models::model_loader_ptr low_res_model;
    models::model_loader_ptr high_res_model;
};

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

class terrains : public lights::object_lighting,
        public renderable::renderable_object
{
public:
    terrains(shaders::my_small_shaders* game_shader);
    /*
     * Load a terrain model, the user might provide its
     * own identificator. If not, a new unique one will be created
     */
    long load_terrain(const std::string& model_filename,
                      const glm::vec3& color,
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
    /*
     * Rendering functions
     */
    void prepare_for_render() override;
    void render() override;
    void clean_after_render() override;

    //Factory
    static terrains_ptr create(shaders::my_small_shaders* shader) {
        return std::make_shared<terrains>(shader);
    }

    const std::vector<terrain_lot>& get_lots() const;
    void mouse_hoover( const types::ray_t& dir );
    /*
     * This is where the camera is pointing, the center of
     * the view field. The idea is to draw only a certain
     * amount of lots which are in the visible area.
     */
    void set_view_center( const glm::vec2& pos,
                          const GLfloat distance );
private:
    shaders::my_small_shaders* shader;
    std::unordered_map<long,lot_models> terrain_container;
    std::unordered_map<long,glm::vec3> default_colors;
    std::set< long > used_ids; //To make sure that all the Ids's are unique.

    GLfloat lot_size;
    std::vector<terrain_lot> terrain_map;
    glm::vec2 view_center;
    /*
     * This variable specify how far from the view_center
     * lots should be rendered
     */
    GLfloat   field_of_view_distance;
    void update_rendr_quality( long rendr_mesh_cnt );

    void unselect_highlighted_lot();
    void select_highlighted_lot( const glm::vec3& lot );
    bool is_highlighted(const glm::vec2 &lot ) const;
    glm::vec3 highlight_lot_color( const glm::vec3& color ) const;
    glm::vec3 highlighted_lot;//With a little brighter color

    rendr_eng_data rendering_data;
    long generate_unique_id();
};

}

#endif
