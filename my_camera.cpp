#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace opengl_play
{

camera_obj my_camera::create_camera(glm::vec3 pos, glm::vec3 target)
{
	return std::make_shared<my_camera>(pos,target);
}

my_camera::my_camera(glm::vec3 position, glm::vec3 target) :
	cam_front( glm::normalize(target) ),
	cam_pos( glm::normalize(position) )
{
	cam_up = glm::vec3( 0.0, 1.0, 0.0 );//Point upward
	pitch = 0;
	yaw = 0;
}

void my_camera::update_cam_view()
{
	cam_dir = glm::normalize( cam_pos - cam_front );
	//Calculate the perperndicular vector
	cam_right = glm::normalize( glm::cross( cam_up, cam_dir ));
	cam_view = glm::lookAt( cam_pos, cam_pos + cam_front, cam_up );
}

void my_camera::move_camera(mov_direction direction, GLfloat speed)
{
	switch(direction) {
	case mov_direction::right:
		cam_pos += glm::normalize( glm::cross(cam_front,cam_up) ) * speed;
		break;
	case mov_direction::left:
		cam_pos -= glm::normalize( glm::cross(cam_front,cam_up) ) * speed;
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
	pitch += pitch_off;
	yaw += yaw_off;
	if(pitch > 89)
		pitch = 89;
	if(pitch < -89)
		pitch = -89;
	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cam_front = glm::normalize(front);
	update_cam_view();
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

void my_camera::set_target(glm::vec3 pos)
{
	cam_front = pos;
	update_cam_view();
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

}
