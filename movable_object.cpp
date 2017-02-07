#include "movable_object.hpp"
#include "logger/logger.hpp"

namespace movable_object
{

movable_object::movable_object() :
	model{ glm::mat4() },
	current_position{ glm::vec3() },
	current_yaw{ 0 },
	current_pitch{ 0 },
	current_roll{ 0 },
	current_scale{ 1.0 }
{

}


void movable_object::update_model()
{
	if( update_needed ) {
		model = glm::mat4();

		model = glm::rotate(model,
					current_yaw,
					glm::vec3(0.0,1.0,0.0));

		model = glm::rotate(model,
					current_pitch,
					glm::vec3(1.0,0.0,0.0));

		model = glm::rotate(model,
					current_roll,
					glm::vec3(0.0,0.0,1.0));

		model = glm::translate(model,
					current_position);

		model = glm::scale(model,
					glm::vec3(current_scale,
							  current_scale,
							  current_scale));

		update_needed = false;
	}
}

void movable_object::set_position(const glm::vec3 &position)
{
	current_position = position;
	update_needed = true;
}

glm::vec3 movable_object::get_position()
{
	return current_position;
}

void movable_object::set_yaw(GLfloat yaw)
{
	current_yaw = yaw;
	update_needed = true;
}

void movable_object::set_pitch(GLfloat pitch)
{
	current_pitch = pitch;
	update_needed = true;
}

void movable_object::set_roll(GLfloat roll)
{
	current_roll = roll;
	update_needed = true;
}

void movable_object::set_scale(GLfloat scale)
{
	current_scale = scale;
	update_needed = true;
}

glm::mat4 movable_object::get_model_matrix()
{
	update_model();
	return model;
}

}
