#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace opengl_play
{

glm::vec3 get_vec_3( const glm::mat4& mat ) {
	return glm::vec3( mat[3].x, mat[3].y, mat[3].z);
}

camera_obj my_camera::create_camera(glm::vec3 pos, glm::vec3 target)
{
	return std::make_shared<my_camera>(pos,target);
}

my_camera::my_camera(glm::vec3 position, glm::vec3 target) :
	cam_front( glm::normalize( target - position ) ),
	target_to_follow{ nullptr },
	mode{ camera_mode::space_mode }
{
	set_position( position );
	update_angles();

	cam_up = glm::vec3( 0.0, 0.0, cos( glm::degrees( current_pitch ) ) );//Point upward
	cam_right = glm::normalize( glm::cross( cam_front, cam_up) );

	update_cam_view();

	//Will be initialized when needed.
	rotation_angle = -1;
}


bool my_camera::eagle_mode(bool is_set)
{
	bool old = ( mode == camera_mode::eagle_mode );
	if( is_set ) {
		/*
		 * top/down and left/right will move
		 * over Y and X
		 */
		update_angles();
		mode = camera_mode::eagle_mode;
		cam_up = glm::vec3(
			sin( glm::radians( current_yaw ) ), //x
			(current_position.y < 0 ? 1 : +1 ) * cos( glm::radians( current_yaw ) ), //y
			0.0); //z

		cam_right = glm::normalize( glm::cross( cam_front,
												glm::vec3(0.0,0.0,1.0)) );
		cam_right.z = 0;
		update_cam_view();
	} else {
		mode = camera_mode::space_mode;
	}
	return old;
}

void my_camera::rotate_around(GLfloat amount)
{
	if( amount == 0 ) {
		WARN1("rotate_around with argument 0 make no sense!");
		return;
	}
	/*
	 * Calculate the target position, it is
	 * the point at Z=0 in the direction where
	 * the camera is looking at: cam_front
	 * TODO: Improve..
	 */
	glm::vec3 cam_pos = get_position();
	glm::vec3 target = cam_pos;
	GLfloat l = 0,
			r = 1024; //Hopefully is big enough!
	while( l < r ) {
		GLfloat mid = ( r + l ) / 2;
		target = cam_pos + cam_front * mid;
		if( glm::abs( target.z ) <= 0.00001 ) {
			//Looks like 0 :)
			break;
		}
		if( target.z > 0 ) {
			l = mid + 0.0001;
		} else {
			r = mid - 0.0001;
		}
		++t;
	}
	//make sure it's zero and not some very small value :)
	target.z = 0.0f;
	//When rotating the distance do not change
	GLfloat distance = glm::distance( glm::vec3(cam_pos.x,cam_pos.y,0.0),
									  target );

	//Eventually set the initial rotation angle
	if( rotation_angle < 0 ) {
		glm::vec3 deltas = glm::vec3(cam_pos.x,cam_pos.y,cam_pos.z) - target;
		rotation_angle = glm::degrees( std::acos( deltas.x / distance ) );
		if( cam_pos.y < 0 ) {
			rotation_angle = 360 - rotation_angle;
		}
		LOG1("Initial value of the rotation angle ",
			 rotation_angle);
	}
	//Make sure that 0<rotation_angle<360
	rotation_angle += amount;
	if( rotation_angle >= 359.99 ) {
		rotation_angle = 0.01;
	}
	if( rotation_angle <= 0 ) {
		rotation_angle = 359.99;
	}
	/*
	 * Calculate the new position on the base
	 * of the amount of rotation
	 */
	GLfloat angle = glm::radians( rotation_angle );

	glm::vec3 new_pos(
				target.x + cos( angle ) * distance,
				target.y + sin( angle ) * distance,
				cam_pos.z
				);
	/*
	 * Update camera vectors to make sure
	 * we point in the right direction
	*/
	cam_front = glm::normalize( target - new_pos );

	cam_up = cam_front;
	cam_up.z = 0;

	cam_right = glm::normalize( glm::cross( cam_front,
								glm::vec3( 0.0, 0.0, 1.0 ) ) );

	set_position( new_pos );
	update_angles();
}

void my_camera::update_cam_view()
{
	cam_view = glm::lookAt( current_position, current_position + cam_front, cam_up );
}

bool my_camera::move(mov_direction direction, GLfloat amount)
{
	bool ret = true;
	switch(direction) {
	case mov_direction::right:
		current_position += cam_right * amount;
		break;
	case mov_direction::left:
		current_position -= cam_right * amount;
		break;
	case mov_direction::top:
		current_position +=  cam_up * amount;
		break;
	case mov_direction::down:
		current_position -= cam_up * amount;
		break;
	case mov_direction::forward:
		current_position += cam_front * amount;
		break;
	case mov_direction::backward:
		current_position -= cam_front * amount;
		break;
	default:
		ERR("my_camera::move: Unknow direction, ",
			static_cast<int>(direction));
		ret = false;
		break;
	}
	if( ret ) {
		update_cam_view();
	}
	return ret;
}

void my_camera::modify_angle(mov_angles angle,GLfloat amount)
{
	/*
	 * If the camera is following a target then
	 * is not possible to perform rotation since
	 * this would mean not following anymore the target
	 */
	if( nullptr == target_to_follow )
	{
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
			ERR("my_camera::modify_angle: Invalid angle, type:",
				static_cast<int>(angle));
			return;
		}

		if( current_yaw >= 359.99 ) current_yaw = 0.01;
		if( current_yaw <= 0.0 ) current_yaw = 359.99;
		if( current_roll >= 359.99 ) current_roll = 0.01;
		if( current_roll <= 0.0 ) current_roll = 359.99;
		if( current_pitch >= 89.99 ) current_pitch = 89.99;
		if( current_pitch <= -89.99 ) current_pitch = -89.99;

		cam_front.x += sin( glm::radians( current_yaw ) ) * cos( glm::radians( current_pitch ) );
		cam_front.y += cos( glm::radians( current_yaw ) ) * cos( glm::radians( current_pitch ) );
		cam_front.z += sin( glm::radians( current_pitch ) );

		cam_front = glm::normalize( cam_front );

		cam_right = glm::normalize( glm::cross( cam_front, cam_up ) );

		if( mode == camera_mode::space_mode ) {
			cam_up = glm::normalize( glm::cross( cam_right, cam_front ) );
		}

		update_cam_view();
	}
}

/*
 * Those Euler coordinates can be extracted
 * from the current direction vector
 */
void my_camera::update_angles()
{
	current_pitch = glm::degrees( asin( cam_front.z ) );
	current_yaw = glm::degrees( atan2( cam_front.x, cam_front.y) );
	current_yaw = current_yaw > 0 ? current_yaw : 360 + current_yaw;
}

glm::mat4 my_camera::get_view()
{
	return cam_view;
}

void my_camera::set_position(const glm::vec3& pos)
{
	current_position = pos;
	update_cam_view();
}

void my_camera::set_target(movable::mov_obj_ptr object)
{
	target_to_follow = object;
}

GLfloat my_camera::get_dist_from_target()
{
	if( nullptr != target_to_follow ) {
		return glm::distance( target_to_follow->get_position(),
							  current_position );
	}
	return -1;
}

void my_camera::follow_target()
{
	if( nullptr != target_to_follow ) {
		auto position = get_position();
		auto model_mtx = target_to_follow->get_model_matrix();
		model_mtx = glm::translate( model_mtx,
									glm::vec3(
									-follow_opt.camera_pan,
									-follow_opt.camera_tilt,
									-follow_opt.target_max_distance) );
		auto new_cam_pos = get_vec_3( model_mtx );
		glm::vec3 delta = new_cam_pos - position;
		delta /= 20;
		position += delta;

		set_position( position );

		//Point somewhere in front of the object, not at the
		//object itself
		model_mtx = target_to_follow->get_model_matrix();
		model_mtx = glm::translate( model_mtx, glm::vec3(0.0,0.0,50.0) ); //Should be an option
		cam_front = glm::normalize( get_vec_3( model_mtx ) - position );
		update_angles();
		update_cam_view();
	}
}

glm::vec3 my_camera::get_camera_front()
{
	return cam_front;
}

}
