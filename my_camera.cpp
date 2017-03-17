#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace opengl_play
{

static
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
	cam_up = glm::vec3( 0.0, 1.0, 0.0 );//Point upward
	cam_right = glm::normalize(glm::cross(cam_front,glm::vec3(0.0,1.0,0.0)));

	update_angles();
	update_cam_view();
}


bool my_camera::eagle_mode(bool is_set)
{
	bool old = ( mode == camera_mode::eagle_mode );
	if( is_set ) {
		mode = camera_mode::eagle_mode;
	} else {
		mode = camera_mode::space_mode;
	}
	return old;
}

void my_camera::rotate_around(GLfloat amount)
{
	std::cout<<amount<<std::endl;
	if( amount == 0 ) {
		WARN1("rotate_around with argument 0 make no sense!");
		return;
	}
	glm::vec3 cam_pos = get_position();
	glm::vec3 target = cam_pos;
	while( target.y > 0 ) {
		target += cam_front * 0.01f;
	}
	target.y = 0;
	GLfloat distance = glm::distance( glm::vec3(cam_pos.x,0.0,cam_pos.z),
									  target );
	rotation_angle += amount;
	if( rotation_angle >= 359.99 ) rotation_angle = 0.01;
	GLfloat angle = glm::radians( rotation_angle );
	glm::vec3 new_pos(
				target.x + cos( angle ) * distance,
				cam_pos.y,
				target.z + sin( angle ) * distance
				);
	cam_front = glm::normalize( target - new_pos );
	cam_right = glm::normalize( glm::cross( cam_front, cam_up ) );
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
		current_position += cam_front * amount;
		break;
	case mov_direction::down:
		current_position -= cam_front * amount;
		break;
	case mov_direction::forward:
		current_position -= glm::cross( cam_right, cam_up ) * amount;
		break;
	case mov_direction::backward:
		current_position += glm::cross( cam_right, cam_up ) * amount;
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

		cam_front.x += cos(glm::radians(current_pitch)) * cos(glm::radians(current_yaw));
		cam_front.y += sin(glm::radians(current_pitch));
		cam_front.z += cos(glm::radians(current_pitch)) * sin(glm::radians(current_yaw));

		cam_front = glm::normalize( cam_front );
		cam_right = glm::normalize(glm::cross( cam_front,
											   glm::vec3(cos(glm::radians(current_roll)),
														 sin(glm::radians(current_roll)),
														 0.0) ));

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
	GLfloat temp = glm::dot( cam_front, glm::vec3(0.0,1.0,0.0) );
	temp = glm::degrees( asin( cam_front.y ) ); //glm::degrees(acos(temp)); Alternative.
	current_pitch = temp;
	temp = glm::degrees( atan2( cam_front.x, cam_front.z) );
	current_yaw = 90 - temp;
	if( current_yaw <= 0 ){
		current_yaw += 360;
	}
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
