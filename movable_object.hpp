#ifndef MOVABLE_OBJECT_HPP
#define MOVABLE_OBJECT_HPP
#include "headers.hpp"
#include <list>
#include <vector>

namespace movable_object
{

class movable_object;
using mov_obj_ptr = std::shared_ptr<movable_object>;

/*
 * DO NOT CHANGE the order of the element
 * in this enum!!
 */
enum class mov_angles
{
	roll,
	pitch,
	yaw
};

enum class mov_direction
{
	left,
	right,
	top,
	down,
	forward,
	backward,
	rot_yaw,
	rot_pitch,
	rot_roll
};

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
public:
	movable_object();
	void set_position(const glm::vec3& position);
	glm::vec3 get_position();
	void set_yaw(GLfloat yaw);
	void set_pitch(GLfloat pitch);
	void set_roll(GLfloat roll);
	void set_scale(GLfloat scale);
	//Roll/Pitch/Yaw the object for the given
	//amount, it might be +-
	void modify_angle(mov_angles angle,GLfloat amount);
	/*
	 * move will attempt to move the movable_object
	 * in certain direction, if any changes was made
	 * to the position the function return true
	 */
	bool move(mov_direction direction, GLfloat amount);
	glm::mat4 get_model_matrix();
};

using key_code_t = int;
using scan_code_t = int;
using act_code_t = int;

/*
 * direction_details is used to
 * determine the direction and the
 * intensity of the movement towards
 * that direction
 */
struct direction_details
{
	mov_direction direction;
	GLfloat speed;
};

using mov_key_mapping = std::pair<key_code_t,direction_details>;
using key_mapping_vec = std::vector<mov_key_mapping>;

/*
 * This class process the keyboard and mouse
 * input and trigger the proper movement
 * for all the registered movable_objects for
 * the given movement.
 */
class object_movement_processor
{
public:
	object_movement_processor();
	/*
	 * Certain keys are mapped to movement
	 * actions, those actions are used
	 * to manipulate the position of the registered
	 * movable_objects
	 */
	void keyboard_input(key_code_t key,
			scan_code_t scan_code,act_code_t action);
	/*
	 *  As for the keyboard input, certain movable_objects
	 * might be moved by mouse inputs, for example
	 * the pitch or yaw might be changed.
	 */
	void mouse_movement(GLdouble new_x,GLdouble new_y);
	/*
	 * Function which process the movements,
	 * needs to be called once for every frame
	 */
	void process_movements();
	/*
	 * Register and unregister movable_objects,
	 * the key_mapping argument provide the proper
	 * mapping that will trigger the change of
	 * positioning of the movable_object
	 */
	void register_movable_object(mov_obj_ptr obj,
						key_mapping_vec key_mapping);

private:
	enum class key_status_t
	{
		pressed,
		not_pressed
	};

	std::vector<key_status_t> key_status;
	using dir_vector = std::vector<direction_details>;
	using obj_dir_map = std::map<mov_obj_ptr, dir_vector>;
	std::map<key_code_t,obj_dir_map> kb_map_mapping;
};

}

#endif
