#ifndef MY_CAMERA_HPP
#define MY_CAMERA_HPP

#include "headers.hpp"

namespace opengl_play
{

class my_camera;

using camera_obj = std::shared_ptr<my_camera>;

class my_camera
{
	glm::vec3 cam_target,
			cam_position,
			cam_dir, //z
			cam_x_axis,
			cam_y_axis;
	glm::mat4 cam_view;
public:
	my_camera(glm::vec3 position,glm::vec3 target);
	void update_cam_view();
	void move_camera(mov_direction direction, GLfloat speed);
	glm::mat4 get_view();
	void set_position(glm::vec3 pos);
	void set_target(glm::vec3 pos);
	static camera_obj create_camera(glm::vec3 pos,glm::vec3 target);
};

}

#endif //MY_CAMERA_HPP
