#include "movable_object.hpp"
#include "logger/logger.hpp"
#include <algorithm>

namespace movable
{

//////////////////////////////////////
/// movable_object implementation
/////////////////////////////////////

movable_object::movable_object() :
	model{ glm::mat4() },
	current_position{ glm::vec3() },
	current_yaw{ 0 },
	current_pitch{ 0 },
	current_roll{ 90 },
	current_scale{ 1.0 }
{

}

void movable_object::set_position(const glm::vec3 &position)
{
	glm::vec3 translation_vector = position - current_position;
	model = glm::translate(model,
				translation_vector);
	current_position = position;
}

glm::vec3 movable_object::get_position()
{
	return current_position;
}

void movable_object::set_yaw(GLfloat yaw)
{
	modify_angle(mov_angles::yaw, current_yaw - yaw);
	current_yaw = yaw;
}

void movable_object::set_pitch(GLfloat pitch)
{
	modify_angle(mov_angles::pitch, current_pitch - pitch);
	current_pitch = pitch;
}

void movable_object::set_roll(GLfloat roll)
{
	modify_angle(mov_angles::roll, current_roll - roll);
	current_roll = roll;
}

void movable_object::set_scale(GLfloat scale)
{
	current_scale = scale;
}

GLfloat movable_object::get_yaw()
{
	return current_yaw;
}

GLfloat movable_object::get_pitch()
{
	return current_pitch;
}

GLfloat movable_object::get_roll()
{
	return current_roll;
}

void movable_object::modify_angle(mov_angles angle,
						GLfloat amount)
{
	static glm::vec3 rotation_angles[3] = {
		{0.0,0.0,1.0},
		{1.0,0.0,0.0},
		{0.0,1.0,0.0}
	};
	model = glm::rotate(model,
						glm::radians(amount),
						rotation_angles[static_cast<int>(angle)]);
	switch( angle ) {
	case mov_angles::pitch:
		current_pitch += amount;
		break;
	case mov_angles::yaw:
		current_yaw += amount;
		break;
	case mov_angles::roll:
		current_roll += amount;
		break;
	default:
		ERR("modify_angle: Unknow angle!");
	}
}

bool movable_object::move(mov_direction direction,
						GLfloat amount)
{
	if( amount < 0 ) {
		ERR("movable_object::move: amount must be >= 0");
		return false;
	}
	glm::vec3 translation_vector;
	bool ret = true;

	switch( direction ) {
	case mov_direction::right:
		amount *= -1;
	case mov_direction::left:
		translation_vector.x = amount;
		break;
	case mov_direction::down:
		amount *= -1;
	case mov_direction::top:
		translation_vector.y = amount;
		break;
	case mov_direction::backward:
		amount *= -1;
	case mov_direction::forward:
		translation_vector.z = amount;
		break;
	default:
		WARN2("movable_object::move: Use modify_angle to change the angles!");
		ret = false;
		break;
	}
	if( true == ret ) {
		model = glm::translate( model,
								translation_vector );
		current_position = glm::vec3(model[3].x,model[3].y,model[3].z);
	}
	return ret;
}

glm::mat4 movable_object::get_model_matrix()
{
	return model;
}

movement_mapping &movable_object::get_movement_setup()
{
	return movement_setup;
}

//////////////////////////////////////
/// object_movement_processor implementation
/////////////////////////////////////

object_movement_processor::object_movement_processor() :
	last_mouse_x_position{ -1 },
	last_mouse_y_position{ -1 }
{
	LOG1("Creating object_movement_processor");
	key_status.resize(1024);
	std::fill(key_status.begin(),key_status.end(),key_status_t::not_pressed);
}

void object_movement_processor::mouse_input(GLdouble new_x,
								GLdouble new_y)
{
	static bool first_input{ true };
	if( false == first_input ) {
		GLdouble x_delta = last_mouse_x_position - new_x,
				y_delta = last_mouse_y_position - new_y;

		if( x_delta > 0 ) {
			mouse_status[ mouse_movement_types::yaw_increase ] += x_delta;
		} else if( x_delta < 0 ){
			mouse_status[ mouse_movement_types::yaw_decrease ] += -x_delta;
		}

		if( y_delta > 0 ) {
			mouse_status[ mouse_movement_types::pitch_increse ] += y_delta;
		} else if( y_delta < 0 ){
			mouse_status[ mouse_movement_types::pitch_decrease ] += -y_delta;
		}
	} else {
		first_input = false;
	}
	last_mouse_x_position = new_x;
	last_mouse_y_position = new_y;
}

void object_movement_processor::keyboard_input(int key,
								int scan_code,
								int action)
{
	while( key >= key_status.size() ) {
		//This should rather not happen
		std::size_t cur_size = key_status.size();
		ERR("key_status is too small: ",
			cur_size,", increasing size!");
		key_status.resize(cur_size * 2, key_status_t::not_pressed);
	}
	if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
		key_status[ key ] = key_status_t::pressed;
		process_speed_selectors( key );
	} else if( action == GLFW_RELEASE ) {
		key_status[ key ] = key_status_t::not_pressed;
	}
}

void object_movement_processor::process_movements()
{
	/*
	 * Process keyboard movements
	 */
	process_impl_keyboard();
	/*
	 * Process mouse movements
	 */
	process_impl_mouse();
	/*
	 * Are there movement to perform
	 * due to tracking setup?
	 */
	object_tracking.process_tracking();
}

void object_movement_processor::register_movable_object(mov_obj_ptr obj,
									const key_mapping_vec& key_mapping)
{
	movement_mapping& mov_setup = obj->get_movement_setup();
	for( auto& key : key_mapping )
	{
		mov_setup[ key.second.direction ] = key.second;
		keyb_mapping[ key.first ][ obj ].push_back( key.second.direction );
	}
}

void object_movement_processor::register_movable_object(mov_obj_ptr obj,
									const mouse_mapping_vec &mapping)
{
	movement_mapping& mov_setup = obj->get_movement_setup();
	for( auto& mouse_mov : mapping )
	{
		mov_setup[ mouse_mov.second.direction ] = mouse_mov.second;
		mouse_mapping[ mouse_mov.first ][ obj ].push_back( mouse_mov.second.direction );
	}
}

void object_movement_processor::unregister_movable_object(mov_obj_ptr obj)
{
	for( auto& elem : keyb_mapping ) {
		elem.second.erase(obj);
	}
}

void object_movement_processor::register_speed_selectors(mov_obj_ptr obj,
									const speed_selector &selector)
{
	for( auto& elem : selector ) {
		speed_selectors[ elem.key ][ obj ].push_back( selector );
	}
}

/*
 * Verify whether the pressed key is not
 * a trigger for a speed selection. If that's the case,
 * the modify the current selector accordingly
 */
void object_movement_processor::process_speed_selectors( key_code_t pressed_key )
{
	/*
	 * Anybody registered a selector for this key?
	 */
/*	auto it = speed_selectors.find( pressed_key );
	if( it != speed_selectors.end() ) {
		//Ok, pick the selector for each object
		for( auto& objects : it->second ) {
			mov_obj_ptr obj = objects.first;
			for( auto& sel : objects.second ) {
				auto key_map = keyb_mapping.find( sel.key );
			}
		}
	}*/
}


tracking_processor &object_movement_processor::tracking()
{
	return object_tracking;
}

void object_movement_processor::process_impl_keyboard()
{
	for( int key = 0 ; key < key_status.size() ; ++key ) {
		if( key_status[ key ] == key_status_t::pressed ) {
			/*
			 * the key 'key' is pressed, check whether anybody
			 * has registered a movable_object to perform
			 * a movement if this key is pressed.
			 */
			auto it = keyb_mapping.find( key );
			if( it != keyb_mapping.end() ) {
				/*
				 * For each registered object, perform the
				 * proper movement
				 */
				trigger_proper_movement(it->second, 1.0);
			}
		}
	}
}

void object_movement_processor::process_impl_mouse()
{
	for( auto& movement : mouse_status ) {
		if( movement.second != 0 ) {
			GLfloat amount = movement.second / 50;
			//Is anybody registered for this movement?
			if( mouse_mapping[ movement.first ].empty() ) {
				movement.second = 0;//Nope? Clean..
			} else {
				trigger_proper_movement( mouse_mapping[ movement.first ], amount );

				movement.second = std::max<GLfloat>( movement.second - amount, 0.0 );
				/*
				 * The bigger is this constant (1.0)
				 * then more softly the object will terminate
				 * is movement.
				 */
				if( movement.second < 1.0 ) {
					movement.second = 0;
				}
			}
		}
	}
}

void object_movement_processor::trigger_proper_movement(obj_dir_map& dir_map,
													GLfloat speed_amount)
{
	for( auto& movable : dir_map ) {
		mov_obj_ptr obj = movable.first;
		movement_mapping& mov_setup = obj->get_movement_setup();
		for( auto& dir : movable.second ) {
			auto sp_setup = mov_setup[ dir ];
			GLfloat speed = sp_setup.speed[ sp_setup.current_speed ] * speed_amount;
			switch( dir ) {
			case mov_direction::left:
			case mov_direction::right:
			case mov_direction::top:
			case mov_direction::down:
			case mov_direction::forward:
			case mov_direction::backward:
				obj->move(dir, speed);
				break;
			case mov_direction::yaw_dec:
				speed *= -1;
			case mov_direction::yaw_inc:
				obj->modify_angle(mov_angles::yaw, speed);
				break;
			case mov_direction::pitch_dec:
				speed *= -1;
			case mov_direction::pitch_inc:
				obj->modify_angle(mov_angles::pitch, speed);
				break;
			case mov_direction::roll_dec:
				speed *= -1;
			case mov_direction::roll_inc:
				obj->modify_angle(mov_angles::roll, speed);
				break;
			default:
				ERR("trigger_proper_movement: Unrecognize movement direction");
				break;
			}
		}
	}
}

//////////////////////////////////////
/// tracking_processor implementation
/////////////////////////////////////

bool tracking_processor::new_tracking(mov_obj_ptr target,
							mov_obj_ptr object,
							GLfloat distance_threashold)
{
	LOG1("new_tracking: New tracking setup!");
	if( tracking_data.find( object ) != tracking_data.end() ){
		WARN1("tracking_processor::new_tracking: Cannot add the same object twice!");
		return false;
	}
	glm::vec3 target_pos = target->get_position(),
			object_pos = object->get_position();
	tracking_info new_track = {
		target,
		object,
		target_pos,
		object_pos,
		distance_threashold,
	};
	auto ret = tracking_data.insert({ object, new_track });
	return ret.second;
}

void tracking_processor::process_tracking()
{
	for( auto& entry : tracking_data ) {
		auto target_pos = entry.second.target->get_position();
		auto object_pos = entry.second.object->get_position();
		GLfloat current_distance = glm::distance( target_pos, object_pos );
		//We shall follow the target which is getting too far
		if( current_distance > entry.second.distance_threshold ) {
			glm::vec3 dir_vector = target_pos - object_pos;
			dir_vector /= 100;
			object_pos = object_pos + dir_vector;
			entry.second.object->set_position( object_pos );
		}
		entry.second.last_target_position = target_pos;
		entry.second.last_object_position = object_pos;
		entry.second.last_recorded_distance = current_distance;
	}
}

GLfloat tracking_processor::get_dist_from_target(mov_obj_ptr object)
{
	auto it = tracking_data.find(object);
	if( it != tracking_data.end() ) {
		return it->second.last_recorded_distance;
	}
	return -1;
}

}
