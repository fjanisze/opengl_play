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
	cam_pos( position ),
	target_to_follow{ nullptr }
{
	cam_up = glm::vec3( 0.0, 1.0, 0.0 );//Point upward
	cam_right = glm::normalize(glm::cross(cam_front,glm::vec3(0.0,1.0,0.0)));

	update_angles();
	update_cam_view();
}

void my_camera::update_cam_view()
{
	cam_view = glm::lookAt( cam_pos, cam_pos + cam_front, cam_up );
}

void my_camera::move_camera(mov_direction direction, GLfloat speed)
{
	switch(direction) {
	case mov_direction::right:
		cam_pos += cam_right * speed;
		break;
	case mov_direction::left:
		cam_pos -= cam_right * speed;
		break;
	case mov_direction::top:
		cam_pos += cam_front * speed;
		break;
	case mov_direction::down:
		cam_pos -= cam_front * speed;
		break;
	default:
		break;
	}
	update_cam_view();
}

void my_camera::rotate_camera(GLdouble pitch_off, GLdouble yaw_off)
{
	/*
	 * If the camera is following a target then
	 * is not possible to perform rotation since
	 * this would mean not following anymore the target
	 */
	if( nullptr == target_to_follow )
	{
		pitch += pitch_off;
		yaw += yaw_off;
		if( yaw >= 359.9 ) yaw = 0.1;
		if( yaw <= 0.0 ) yaw = 360;
		if( pitch >= 89.9 ) pitch = 89.9;
		if( pitch <= -89.9 ) pitch = -89.9;

		cam_front.x += cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		cam_front.y += sin(glm::radians(pitch));
		cam_front.z += cos(glm::radians(pitch)) * sin(glm::radians(yaw));

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
	pitch = temp;
	temp = glm::degrees(atan2(cam_front.x,cam_front.z));
	yaw = 90 - temp;
	if( yaw <= 0 ){
		yaw += 360;
	}
}

glm::mat4 my_camera::get_view()
{
	return cam_view;
}

void my_camera::set_position(glm::vec3 pos)
{
	cam_pos = pos;
	update_cam_view();
}

void my_camera::set_target(movable_object::mov_obj_ptr object)
{
	target_to_follow = object;
}

void my_camera::follow_target()
{
	if( nullptr != target_to_follow ) {
		cam_front = glm::normalize( target_to_follow->get_position() - cam_pos );
		update_angles();
		update_cam_view();
	}
}

GLdouble my_camera::get_camera_yaw()
{
	return yaw;
}

GLdouble my_camera::get_camera_pitch()
{
	return pitch;
}

glm::vec3 my_camera::get_camera_pos()
{
	return cam_pos;
}

glm::vec3 my_camera::get_camera_front()
{
	return cam_front;
}

}
