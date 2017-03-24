#include <terrains.hpp>
#include <logger/logger.hpp>

namespace terrains
{

terrains::terrains(shaders::my_small_shaders *game_shader) :
    object_lighting( game_shader ),
    shader{ game_shader }
{
    LOG1("Creating terrains::terrains");
	unselect_highlighted_lot();
}

long terrains::load_terrain(const std::string &model_filename,
                            const glm::vec3 &color,
                            long terrain_id)
{
    LOG1("Loading terrain model: ",
         model_filename,
         ". Provided ID: ",
         terrain_id);
    models::model_loader_ptr new_model = models::model_loader::load(
                model_filename);
    if( terrain_id < 0 ) {
        if( used_ids.empty() ) {
            terrain_id = 1;
        } else {
            /*
             * std::set is sorted, pick the largest
             * one and generate a new unique ID
             */
            terrain_id = *used_ids.rbegin() + 1;
        }
    }
    if( used_ids.insert( terrain_id ).second ) {
        terrain_container[ terrain_id ] = new_model;
        default_colors[ terrain_id ] = color;
        LOG1("New terrain loaded, id: ", terrain_id,
             ". Amount of terrains: ", terrain_container.size() );
        add_renderable( this );
    } else {
        ERR("The provided terrain ID is already in use!");
        terrain_id = -1;
    }

    return terrain_id;
}

bool terrains::load_terrain_map(const terrain_map_t &map,
                                GLfloat lot_size,
                                glm::vec2 central_lot)
{
    if( lot_size <= 0 ){
        ERR("Lot size couldn't be zero!");
        return false;
    }
    if( map.empty() || map[0].empty() ) {
        ERR("The provided terrain map is empty!");
        return false;
    }
    this->lot_size = lot_size;
    /*
     * Process the provided terrain map and
     * generate the proper internal reppresentation
     */
    std::size_t x_size = map[0].size();
    std::vector< terrain_lot > new_terrain_map;
    for( std::size_t y{ 0 } ; y < map.size() ; ++y )
    {
        if( x_size != map[ y ].size() ) {
            //Should be equal for all,the map should be a quad
            ERR("The provided terrain map is not a quad!");
            return false;
        }
        for( std::size_t x{ 0 } ; x < map[y].size() ; ++x )
        {
            terrain_lot new_lot;
            new_lot.terrain_id = map[ y ][ x ];
            new_lot.position = glm::vec2(x - central_lot.x, y - central_lot.y );
            new_lot.model_matrix = glm::translate( new_lot.model_matrix,
                                                   glm::vec3( new_lot.position.x * lot_size,
                                                              new_lot.position.y * lot_size,
                                                              0.0) );
            /*
             * Is the lot model available?
             */
            if( terrain_container.find( new_lot.terrain_id ) == terrain_container.end() )
            {
                WARN1("The model for the terrain ID: ",
                      new_lot.terrain_id," is not loaded..");
            }
            new_terrain_map.push_back( new_lot );
        }
    }
    terrain_map = std::move( new_terrain_map );
	for( auto elem : terrain_map ) {
		std::cout<<elem.position.x<<","<<elem.position.y<<std::endl;
	}
	std::cout<<"After short\n";
	std::sort( terrain_map.begin(), terrain_map.end() );
	for( auto elem : terrain_map ) {
		std::cout<<elem.position.x<<","<<elem.position.y<<std::endl;
	}
    return true;
}

void terrains::prepare_for_render()
{
    shader->use_shaders();
    GLint view_loc = glGetUniformLocation(*shader,"view");
    GLint projection_loc = glGetUniformLocation(*shader,"projection");

    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

    calculate_lighting();
}

void terrains::render()
{
    GLint model_loc = glGetUniformLocation(*shader,"model");

    glm::vec3 last_color;
    for( auto& lot : terrain_map ) {
        auto& model = terrain_container[ lot.terrain_id ];
		auto color = default_colors[ lot.terrain_id ];
		if( is_highlighted( lot.position ) ) {
			color = highlight_color( color );
		}
        if( color != last_color ) {
            //Do not update the color if not needed.
            apply_object_color( color );
            last_color = color;
        }
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr( lot.model_matrix ));
        for( auto&& mesh : model->get_mesh() ) {
            mesh->render( shader );
        }
    }
}

void terrains::clean_after_render()
{

}

const std::vector<terrain_lot> &terrains::get_lots() const
{
    return terrain_map;
}

/*
 * The 'dir' vector specify the proper ray cast
 * of the mouse when moving over the terrain
 */
void terrains::mouse_hoover(const types::ray_t &dir )
{
	unselect_highlighted_lot();
	/*
	 * Find the position with Z=0, we simplify thing
	 * thing by ignoring the Z of the models
	 */
	glm::vec3 target = dir.first;
	GLfloat l = 0,
			r = 1024; //Hopefully is big enough!
	while( l < r ) {
		GLfloat mid = ( r + l ) / 2;
		target = dir.first + dir.second * mid;
		if( glm::abs( target.z ) <= 0.00001 ) {
			//Looks like 0 :)
			break;
		}
		if( target.z > 0 ) {
			l = mid + 0.0001;
		} else {
			r = mid - 0.0001;
		}
	}
	target.z = 0;
	/*
	 * There's a weird offset that we need to fix,
	 * also the size of each lot is lot_size * lot_size
	 */
	target += 1.0f;
	target /= 2.0f;
	target = glm::floor( target );
	select_highlighted_lot( target );
}

void terrains::unselect_highlighted_lot()
{
	/*
	 * An highlighted lot should have Z equal to 0
	 */
	highlighted_lot.z = 1.0f;
}

void terrains::select_highlighted_lot(const glm::vec3 &lot)
{
	highlighted_lot = lot;
	highlighted_lot.z = 0.0f;
}

/*
 * Return true if 'lot' should be highlighted
 */
bool terrains::is_highlighted(const glm::vec2 &lot) const
{
	return highlighted_lot.z == 0.0f &&
			highlighted_lot.x == lot.x &&
			highlighted_lot.y == lot.y;
}

glm::vec3 terrains::highlight_color(const glm::vec3 &color) const
{
	return color * 2.0f;
}



}
