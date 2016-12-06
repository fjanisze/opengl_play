#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace opengl_play
{

camera_obj my_camera::create_camera(glm::vec3 pos, glm::vec3 target)
{
	return std::make_shared<my_camera>(pos,target);
}

my_camera::my_camera(glm::vec3 position, glm::vec3 target) :
	cam_target( target ),
	cam_position( position )
{

}

void my_camera::update_cam_view()
{
	cam_y_axis = glm::vec3( 0.0, 1.0, 0.0 );//Point upward
	cam_dir = glm::normalize( cam_position - cam_target );
	//Calculate the perperndicular vector
	cam_x_axis = glm::normalize( glm::cross( cam_y_axis, cam_dir ));
	//Now the real y axis
	cam_y_axis = glm::normalize( glm::cross( cam_dir, cam_x_axis ));
	cam_view = glm::lookAt( cam_position, cam_target, cam_y_axis );
}

void my_camera::move_camera(mov_direction direction, GLfloat speed)
{
	switch(direction) {
	case mov_direction::right:
		cam_position += cam_x_axis * speed;
		break;
	case mov_direction::left:
		cam_position -= cam_x_axis * speed;
		break;
	case mov_direction::top:
		cam_position -= cam_dir * speed;
		break;
	case mov_direction::down:
		cam_position += cam_dir * speed;
		break;
	default:
		break;
	}
	update_cam_view();
}

glm::mat4 my_camera::get_view()
{
	return cam_view;
}

void my_camera::set_position(glm::vec3 pos)
{
	cam_position = pos;
	update_cam_view();
}

void my_camera::set_target(glm::vec3 pos)
{
	cam_target = pos;
	update_cam_view();
}


}
