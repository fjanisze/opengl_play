#ifndef MOVABLE_OBJECT_HPP
#define MOVABLE_OBJECT_HPP
#include "headers.hpp"

namespace movable_object
{

/*
 * Every movable object should inherit from
 * this class, which implements the operations
 * required to move the object (by the proper
 * transformation matrix) in the space
 */
class movable_object
{
protected:
	glm::mat4 model;
	glm::vec3 current_position;
	GLfloat current_yaw,
			current_pitch,
			current_roll,
			current_scale;
	bool update_needed;
	void update_model();
public:
	movable_object();
	void set_position(const glm::vec3& position);
	glm::vec3 get_position();
	void set_yaw(GLfloat yaw);
	void set_pitch(GLfloat pitch);
	void set_roll(GLfloat roll);
	void set_scale(GLfloat scale);
	glm::mat4 get_model_matrix();
};

}

#endif
