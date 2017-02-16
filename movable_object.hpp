#ifndef MOVABLE_OBJECT_HPP
#define MOVABLE_OBJECT_HPP
#include "headers.hpp"
#include <list>
#include <vector>
#include <unordered_map>

namespace movable
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
	virtual void set_position(const glm::vec3& position);
	glm::vec3 get_position();
	virtual void set_yaw(GLfloat yaw);
	virtual void set_pitch(GLfloat pitch);
	virtual void set_roll(GLfloat roll);
	virtual void set_scale(GLfloat scale);
	virtual GLfloat get_yaw();
	virtual GLfloat get_pitch();
	virtual GLfloat get_roll();
	//Roll/Pitch/Yaw the object for the given
	//amount, it might be +-
	virtual void modify_angle(mov_angles angle,GLfloat amount);
	/*
	 * move will attempt to move the movable_object
	 * in certain direction, if any changes was made
	 * to the position the function return true
	 */
	virtual bool move(mov_direction direction, GLfloat amount);
	virtual glm::mat4 get_model_matrix();
};

using key_code_t = int;
using scan_code_t = int;
using act_code_t = int;

/*
 * This class implements the logic
 * which allow objects to follow each other
 */
class tracking_processor
{
public:
	tracking_processor() = default;
	/*
	 * Setup a give object to follow a target object,
	 * the argument distance_threshold provide
	 * the maximum distance that is allowed between
	 * the target and the follower, exceeding this
	 * distance will trigger the tracking.
	 */
	bool new_tracking(mov_obj_ptr target,
					mov_obj_ptr object,
					GLfloat distance_threashold);
	/*
	 * Shall be called at every frame to update
	 * the position of the objects which are
	 * expected to move
	 */
	void process_tracking();
	/*
	 * Return the last recorded distance from
	 * the target for the given object
	 */
	GLfloat get_dist_from_target(mov_obj_ptr object);
private:
	//Information needed to handle the object tracking
	struct tracking_info {
		mov_obj_ptr target,
					object;
		glm::vec3 last_target_position,
				last_object_position;
		GLfloat distance_threshold,
				last_recorded_distance;
		tracking_info() = default;
	};
	std::unordered_map<mov_obj_ptr,tracking_info> tracking_data;
};

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

//Mapping for the keyboard
using mov_key_mapping = std::pair<key_code_t,direction_details>;
using key_mapping_vec = std::vector<mov_key_mapping>;

/*
 * For mapping mouse movements into object
 * movements
 */
enum class mouse_movement_types {
	pitch_increse,
	pitch_decrease,
	yaw_increase,
	yaw_decrease
};

/*
 * That's usefull to pass a mouse_movement_types type
 * to the function modify_angle
 */
constexpr
mov_angles to_mov_angles( const mouse_movement_types& mov )
{
	if( mov == mouse_movement_types::pitch_decrease ||
			mov == mouse_movement_types::pitch_increse )
		return mov_angles::pitch;
	return mov_angles::yaw;
}

//Mapping for the mouse
using mov_mouse_mapping = std::pair<mouse_movement_types,direction_details>;
using mouse_mapping_vec = std::vector<mov_mouse_mapping>;

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
	void mouse_input(GLdouble new_x,GLdouble new_y);
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
	void register_movable_object(mov_obj_ptr obj,
						mouse_mapping_vec mapping);
	void unregister_movable_object(mov_obj_ptr obj);

	tracking_processor& tracking();
private:
	enum class key_status_t
	{
		pressed,
		not_pressed
	};

	//Store the known value of the keys
	std::vector<key_status_t> key_status;
	//Registered mappings
	using dir_vector = std::vector<direction_details>;
	using obj_dir_map = std::map<mov_obj_ptr, dir_vector>;
	std::map<key_code_t,obj_dir_map> keyb_mapping;
	//Registered mapping for the mouse movements
	std::map<mouse_movement_types,obj_dir_map> mouse_mapping;
	GLdouble last_mouse_x_position,
			last_mouse_y_position;
	/*
	 * For each possible mouse movement indicate
	 * how much the mouse moved from the previous
	 * processed position (0 if no movements)
	 */
	std::unordered_map<mouse_movement_types,GLfloat> mouse_status;
	//To enable object tracking
	tracking_processor object_tracking;
};

}

#endif
