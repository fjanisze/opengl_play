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
#include <units.hpp>

namespace game_terrains
{

/*
 * For each lot we store up to two
 * models, one low res (normal res) one high res).
 * For closeup scenes 'terrains' will load the high
 * res model
 */
struct Lot_model_textures
{
    types::color default_color;
    /*
     * If not provided, the high_res_model is the
     * same as the low res..
     */
    models::model_loader::pointer low_res_model;
    models::model_loader::pointer high_res_model;
};

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
class Terrain_lot : public renderer::Renderable
{
public:
    using pointer = std::shared_ptr< Terrain_lot >;

    Terrain_lot( const long model_id,
                 const glm::vec2 unique_position ,
                 const GLfloat lot_altitude );

    /*
     * This is the ID corresponding to one
     * of the loaded terrain models.
     *
     * Changing this ID will cause the lot
     * to be rendered with a different model,
     * in the future this might be used to implement
     * terraforming: A terrain change it appearance due
     * to some gameplay, changing this ID will change
     * the look of the terrain on the map.
     */
    const long terrain_model_id;
    /*
     * This is the unique lot ID for this
     * lot of terrain
     */
    const id_factory< Terrain_lot > id;
    const glm::vec2    position;
    /*
     * The altitude (+Z) of a lot is important
     * to avoid units and other stuff to accidentally
     * end up 'inside' the lot
     */
    const GLfloat      altitude;
    Lot_model_textures textures;
    game_units::Unit::container units;

    /*
     * Rendering function
     */
    void render(shaders::Shader::pointer &shader ) override;
};

/*
 * Handle the game map, which is a matrix of lots
 * of differnet kinds.
 */
class Terrains
{
public:
    using pointer = std::shared_ptr< Terrains >;
    Terrains( renderer::Core_renderer_proxy renderer );
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
    /*
     * Return the lot at the give coordinates
     */
    Terrain_lot::pointer find_lot( const glm::vec2 coord );
    /*
     * Return the currently selected lot, if any
     */
    Terrain_lot::pointer selected_lot();
private:
    renderer::Core_renderer_proxy renderer;

    std::unordered_map< long, Lot_model_textures > terrain_container;

    GLfloat lot_size;
    long get_position_idx( const glm::vec2& pos ) const;
    /*
     * Map a position index to a Terrain_lot
     */
    std::unordered_map< long, Terrain_lot::pointer > terrain_map;
    /*
     * Map a renderable ID to a position index
     */
    std::unordered_map< long , long > rendr_id_to_idx;
    glm::vec2 view_center;
    /*
     * The origins lot is the lot from which
     * the coordinate numbering starts
     */
    glm::vec2 origins_lot;

    /*
     * Return the model matrix for the lot
     * at position pos
     */
    glm::mat4 get_lot_model_matrix( const glm::vec2& pos ) const;
};

}

#endif
