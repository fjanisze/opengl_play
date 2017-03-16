#include "lights.hpp"

namespace lights
{

std::vector<generic_light_ptr> object_lighting::all_lights;

object_lighting::object_lighting(shaders::my_small_shaders * shader) :
	frag_shader{ shader }
{
	LOG1("New object_lighting");
	/*
	 * Hardcoded size! This allow to support
	 * something like 128 lights!
	 */
	light_data_buffer.resize( 1024 ); //TODO, there should be one buffer for ALL, not many copies with identical data (see calculate_lighting)
}

/*
 * Calculate the ambient light color and intensity,
 * load to the proper uniforms the position,color and strength
 * of all the lights in the scene.
 *
 * Those information will be used during the render
 * procedure (by the fragment shader) for all
 * the little_object. (see little_object::render)
 */
void object_lighting::calculate_lighting()
{
	int light_cnt = 0;
	std::size_t current_idx{ 0 };

	for(auto & light : all_lights) {
		auto light_data = light->get_light_data();
		for( auto& entry : light_data ) {
			light_data_buffer[ current_idx++ ] = entry;
			if( current_idx >= 1024 ) {
				ERR("Too much light data to fit!");
				throw std::runtime_error("light_data_buffer too small!!");
			}
		}
		++light_cnt;
	}

	GLint nl = glGetUniformLocation(*frag_shader,
							   "number_of_lights");

	if( nl < 0 )
	{
		ERR("Unable to load the uniform number_of_lights");
	}
	else
	{
		glUniform1i(nl,all_lights.size());
	}

	GLint shader_light_buffer = glGetUniformLocation(*frag_shader,
													 "light_data");

	if( shader_light_buffer < 0 )
	{
		ERR("Unable to load the uniform shader_light_buffer");
	}
	else
	{
		glUniform1fv(shader_light_buffer,
					 current_idx,
					 light_data_buffer.data());
	}
}

void object_lighting::apply_object_color(const glm::vec3 &color)
{
	//Apply the object color
	GLint obj_color_uniform = glGetUniformLocation(*frag_shader,
											   "object_color");
	glUniform3f(obj_color_uniform,
				color.r,
				color.b,
				color.g);
}

//////////////////////////////////////
/// generic_light implementation
/////////////////////////////////////

void generic_light::init_render_buffers() throw (std::runtime_error)
{
	LOG1("init_render_buffers");

	cube_vrtx = std::make_unique<GLfloat[]>(36 * 3);
	std::copy(model_vertices::cube_vertices,
			  model_vertices::cube_vertices + 36 * 3 - 1,
			  cube_vrtx.get());

	light_shader.load_vertex_shader(
				light_shader.read_shader_body("../light_shader.vert"));
	light_shader.load_fragment_shader(
				light_shader.read_shader_body("../light_shader.frag"));

	if(!light_shader.create_shader_program()) {
		ERR("Unable to create the shader program");
		throw std::runtime_error("Failed to create the light_shader.");
	}

	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(GLfloat) * 36 * 3,
				 cube_vrtx.get(),
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,
						  sizeof(GL_FLOAT) * 3,
						  (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
}

std::size_t generic_light::fill_common_light_data()
{
	light_data[0] = static_cast<int>(light_type());
	glm::vec3 pos = get_position();
	light_data[1] = pos.x;
	light_data[2] = pos.y;
	light_data[3] = pos.z;
	auto color_info = get_light_color();
	light_data[4] = color_info.first.r;
	light_data[5] = color_info.first.g;
	light_data[6] = color_info.first.b;
	light_data[7] = color_info.second;
	return 8; //Next valid index
}

generic_light::generic_light()
{
	//Setup the default size for a generic light
	light_data.resize(light_data_size());
}

generic_light::generic_light(glm::vec3 position,
						   glm::vec3 color,
						   GLfloat strength) :
	light_color{ color },
	color_strength{ strength }
{
	set_position(position);
	set_scale(0.5);
	//Setup the default size for a generic light
	light_data.resize(light_data_size());
}

generic_light::~generic_light()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
}

GLfloat generic_light::get_strength()
{
	return color_strength;
}

void generic_light::set_strength(GLfloat strength)
{
	color_strength = strength;
}

std::pair<glm::vec3, GLfloat> generic_light::get_light_color()
{
	return std::make_pair(light_color,color_strength);
}

std::size_t generic_light::light_data_size()
{
	/*
	 * Those are the default values for a generic light
	 */
	return 1 + //type
		   3 + //position
		   3 + //color
			1;  //strength
}

void generic_light::attach_to_object(movable::mov_obj_ptr object)
{
	WARN1("attach_to_object not implemented!");
}

const std::vector<GLfloat>& generic_light::get_light_data()
{
	/*
	 * The format of the data for a generic
	 * light is :
	 * light_type = 1 float (0)
	 * light_pos  = 3 float (x,y,z)
	 * light_color= 3 float (r,b,g)
	 * strength   = 1 float (s)
	 */
	//TODO: Do not always update, just if needed
	fill_common_light_data();
	return light_data;
}


//////////////////////////////////////
/// point_light implementation
/////////////////////////////////////


point_light::point_light(glm::vec3 position,
						   glm::vec3 color,
						   GLfloat strength) :
	generic_light(position,color,strength)
{
	LOG1("Constructor: point_light");
	init_render_buffers();

	//add_renderable(this);
}

point_light::~point_light()
{
	//remove_renderable( this );
}

void point_light::set_transformations(glm::mat4 v,
							glm::mat4 p)
{
	view = v;
	projection = p;
}

void point_light::prepare_for_render()
{
	light_shader.use_shaders();

	GLint obj_color_uniform = glGetUniformLocation(light_shader,
											   "light_color");
	glUniform3f(obj_color_uniform,
				light_color.r,
				light_color.g,
				light_color.b);

	GLint obj_col_strength_uniform = glGetUniformLocation(light_shader,
											   "light_strength");
	glUniform1f(obj_col_strength_uniform,
				color_strength);

	glm::mat4 model = get_model_matrix();

	GLint model_loc = glGetUniformLocation(light_shader,"model");
	GLint view_loc = glGetUniformLocation(light_shader,"view");
	GLint projection_loc = glGetUniformLocation(light_shader,"projection");

	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
}

void point_light::render()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES,0,36);
	glBindVertexArray(0);
}

void point_light::clean_after_render()
{

}

void point_light::rotate_object(GLfloat yaw)
{
	current_yaw = yaw;
}

//////////////////////////////////////
/// directional_light implementation
/////////////////////////////////////

directional_light::directional_light(glm::vec3 direction,
									 glm::vec3 color,
									 GLfloat strength)
{
	set_position(direction);
	light_color = color;
	color_strength = strength;
}

//////////////////////////////////////
/// spot_light and flash_light implementation
/////////////////////////////////////

/*
 * In the constructor the parameter 'direction'
 * is supposed to be the point where the light
 * is pointing, this point is understand by the
 * shader as the offset from the current light
 * position, so to make the whole thing work
 * correctly we need to calculate light_direction
 * as the offset 'from' the current position TO
 * the 'direction' choosen by the user.
 */
spot_light::spot_light(glm::vec3 position,
			glm::vec3 color,
			GLfloat strength,
			glm::vec3 direction,
			GLfloat cut_off_angle,
			GLfloat out_cutoff_angle) :
	generic_light::generic_light(position,color,strength),
	light_direction{ direction - position },
	cut_off{ glm::cos( glm::radians(cut_off_angle) ) },
	out_cutoff{ glm::cos( glm::radians( out_cutoff_angle) ) }
{
	light_data.resize( light_data_size() );
}

void spot_light::attach_to_object(movable::mov_obj_ptr object)
{
	target_obj = object;
}

std::size_t spot_light::light_data_size()
{
	/*
	 * The additional elements are the light direction
	 * plus the two cutoff angles
	 */
	return generic_light::light_data_size() + 5;
}

const std::vector<GLfloat> &spot_light::get_light_data()
{
	std::size_t idx{ 0 };
	if( target_obj == nullptr ) {
		idx = fill_common_light_data();
		light_data[ idx     ] = light_direction.x;
		light_data[ idx + 1 ] = light_direction.y;
		light_data[ idx + 2 ] = light_direction.z;
		idx += 3;
	} else {
		//Calculate the new position and direction
		//on the base of the target model-matrix
		glm::vec3 pos = target_obj->get_position();
		recalculate_light_direction();
		light_data[ idx     ] = static_cast<int>(light_type());
		light_data[ idx + 1 ] = pos.x;
		light_data[ idx + 2 ] = pos.y;
		light_data[ idx + 3 ] = pos.z;
		auto color_info = get_light_color();
		light_data[ idx + 4 ] = color_info.first.r;
		light_data[ idx + 5 ] = color_info.first.g;
		light_data[ idx + 6 ] = color_info.first.b;
		light_data[ idx + 7 ] = color_info.second;
		light_data[ idx + 8 ] = light_direction.x;
		light_data[ idx + 9 ] = light_direction.y;
		light_data[ idx + 10 ] = light_direction.z;
		idx += 11;
	}
	light_data[ idx++ ] = cut_off;
	light_data[ idx ] = out_cutoff;
	return light_data;
}

void spot_light::recalculate_light_direction()
{
	if( target_obj != nullptr ) {
		//Calculate the new position and direction
		//on the base of the target model-matrix
		glm::mat4 model_mtx = target_obj->get_model_matrix();
		glm::vec3 pos = glm::vec3( model_mtx[3].x, model_mtx[3].y, model_mtx[3].z );
		model_mtx = glm::translate( model_mtx,
							  glm::vec3(0.0,0.0,200.0) ); //Far enough?
		light_direction = glm::normalize(
					glm::vec3( model_mtx[3].x, model_mtx[3].y, model_mtx[3].z ) - pos );
	}
}

flash_light::flash_light(opengl_play::camera_obj camera,
				glm::vec3 color,
				GLfloat strength,
				GLfloat cut_off_angle,
				GLfloat out_cutoff_angle) :
	spot_light::spot_light(glm::vec3(0.0), //Do not matter
						   color,
						   strength,
						   glm::vec3(0.0), //Do not matter
						   cut_off_angle,
						   out_cutoff_angle),
	camera_ptr{ camera }
{

}

const std::vector<GLfloat> &flash_light::get_light_data()
{
	/*
	 * Update the camera position and direction on the
	 * basis of the camera position
	 */
	glm::vec3 new_position = camera_ptr->get_position(),
			new_direction = camera_ptr->get_camera_front();
	set_position(new_position);
	light_direction = new_direction;
	return spot_light::get_light_data();
}

}
