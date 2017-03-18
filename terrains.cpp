#include <terrains.hpp>
#include <logger/logger.hpp>

namespace terrains
{

terrains::terrains(shaders::my_small_shaders *game_shader) :
	object_lighting( game_shader ),
	shader{ game_shader }
{
	LOG1("Creating terrains::terrains");
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
		auto& color = default_colors[ lot.terrain_id ];
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

void terrains::check_for_hits(const glm::vec2 &point)
{
	for( auto&& lot : terrain_map )
	{
		glm::vec2 pos( lot.model_matrix[3].x, lot.model_matrix[3].z );
		pos = glm::normalize( pos );
		if( point.x > pos.x && point.x <= pos.x + 1.0f/lot_size ) {
			if( point.y > pos.y && point.y <= pos.y + 1.0f/lot_size) {
				std::cout<<"Selected: "<<lot.terrain_id<<": "<<lot.position.x<<","<<lot.position.y<<std::endl;
			}
		}
	}
}



}
