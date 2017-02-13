#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace opengl_play
{

camera_obj my_camera::create_camera(glm::vec3 pos, glm::vec3 target)
{
	return std::make_shared<my_camera>(pos,target);
}

my_camera::my_camera(glm::vec3 position, glm::vec3 target) :
	cam_front( glm::normalize( target - position ) ),
	target_to_follow{ nullptr }
{
	set_position( position );
	cam_up = glm::vec3( 0.0, 1.0, 0.0 );//Point upward
	cam_right = glm::normalize(glm::cross(cam_front,glm::vec3(0.0,1.0,0.0)));

	update_angles();
	update_cam_view();
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
		default:
			ERR("my_camera::modify_angle: Invalid angle, type:",
				static_cast<int>(angle));
			return;
		}

		if( current_yaw >= 359.99 ) current_yaw = 0.01;
		if( current_yaw <= 0.0 ) current_yaw = 359.99;
		if( current_pitch >= 89.99 ) current_pitch = 89.99;
		if( current_pitch <= -89.99 ) current_pitch = -89.99;

		cam_front.x += cos(glm::radians(current_pitch)) * cos(glm::radians(current_yaw));
		cam_front.y += sin(glm::radians(current_pitch));
		cam_front.z += cos(glm::radians(current_pitch)) * sin(glm::radians(current_yaw));

		cam_front = glm::normalize(cam_front);
		cam_right = glm::normalize(glm::cross(cam_front,glm::vec3(0.0,1.0,0.0)));
		cam_up = glm::normalize(glm::cross(cam_right,cam_front));
		update_cam_view();
	}
}

/*
 * Those Euler coordinates can be extracted
 * from the current direction vector
 */
void my_camera::update_angles()
{
	GLfloat temp = glm::dot(cam_front, glm::vec3(0.0,1.0,0.0));
	temp = glm::degrees(asin(cam_front.y)); //glm::degrees(acos(temp)); Alternative.
	current_pitch = temp;
	temp = glm::degrees(atan2(cam_front.x,cam_front.z));
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
		cam_front = glm::normalize( target_to_follow->get_position() - current_position );
		update_angles();
		update_cam_view();
	}
}

glm::vec3 my_camera::get_camera_front()
{
	return cam_front;
}

}
