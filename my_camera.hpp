#ifndef MY_CAMERA_HPP
#define MY_CAMERA_HPP

#include "headers.hpp"
#include <movable_object.hpp>

namespace opengl_play
{

using movable::mov_angles;
using movable::mov_direction;

class my_camera;

using camera_obj = std::shared_ptr<my_camera>;

class my_camera : public movable::movable_object
{
	glm::vec3 cam_front,
			cam_dir, //z
			cam_right,
			cam_up;
	glm::mat4 cam_view;
	movable::mov_obj_ptr target_to_follow;
	void update_angles();
public:
	my_camera(glm::vec3 position,glm::vec3 target);
	void update_cam_view();
	bool move(mov_direction direction, GLfloat amount) override;
	void modify_angle(mov_angles angle,GLfloat amount) override;
	glm::mat4 get_view();
	void set_position(const glm::vec3& pos) override;
	void set_target(movable::mov_obj_ptr object);
	void follow_target();
	glm::vec3 get_camera_front();
	static camera_obj create_camera(glm::vec3 pos,glm::vec3 target);
};

}

#endif //MY_CAMERA_HPP
