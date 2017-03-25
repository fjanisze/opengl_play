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
private:
    shaders::my_small_shaders* shader;
    std::unordered_map<long,models::model_loader_ptr> terrain_container;
    std::unordered_map<long,glm::vec3> default_colors;
    std::set< long > used_ids; //To make sure that all the Ids's are unique.

    GLfloat lot_size;
    std::vector<terrain_lot> terrain_map;
	void unselect_highlighted_lot();
	void select_highlighted_lot( const glm::vec3& lot );
	bool is_highlighted(const glm::vec2 &lot ) const;
	glm::vec3 highlight_lot_color( const glm::vec3& color ) const;
	glm::vec3 highlighted_lot;//With a little brighter color
};

}

#endif
