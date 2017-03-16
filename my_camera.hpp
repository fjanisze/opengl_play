#ifndef MY_CAMERA_HPP
#define MY_CAMERA_HPP

#include "headers.hpp"
#include <movable_object.hpp>
#include <logger/logger.hpp>
#include <optional>

namespace opengl_play
{

using movable::mov_angles;
using movable::mov_direction;

class my_camera;

using camera_obj = std::shared_ptr<my_camera>;

/*
 * Possible option for the camera setup
 */
enum class camera_opt {
	//Camera target following options
	max_target_distance,
	camera_tilt, //Fixed tilt in respect to the target
	camera_pan, //Fixed pan in respect to the target
};

/*
 * Available camera working modes
 */

enum class camera_mode {
	space_mode, //Angles and vectors updated according to position and orientation
	eagle_mode  //The camera will not update the UP vector, forcing movements parallel to x/z
};

/*
 * Common place for storing all the camera
 * options, with the functions to enable them
 */
struct target_following_options
{
	std::vector<camera_opt> enabled_options;
	GLfloat target_max_distance;
	GLfloat camera_tilt;
	GLfloat camera_pan;
	target_following_options() = default;
};

template<typename V>
using camera_option = std::pair<camera_opt,V>;

template<typename T>
constexpr camera_option<T> new_option(camera_opt opt, std::optional<T>&& value)
{
	return camera_option<T>( opt, value.value_or(0) );
}

class my_camera : public movable::movable_object
{
public:
	my_camera(glm::vec3 position,glm::vec3 target);
	void update_cam_view();
	bool move(mov_direction direction, GLfloat amount) override;
	void modify_angle(mov_angles angle,GLfloat amount) override;
	glm::mat4 get_view();
	void set_position(const glm::vec3& pos) override;
	void set_target(movable::mov_obj_ptr object);
	GLfloat get_dist_from_target();
	/*
	 * Enable the camera to follow the given
	 * movable object
	 */
	void follow_target();
	/*
	 * Provide a certain set of option for the
	 * way the camera follow the movable_object
	 */
	template<typename CUR_OPT,typename...OPT>
	void setup_following_options(CUR_OPT&& opt, OPT&&...remaining);
	glm::vec3 get_camera_front();
	static camera_obj create_camera(glm::vec3 pos,glm::vec3 target);
	/*
	 * When set the camera will maintain a fixed
	 * UP vector, in this way the camera
	 * will emulate the eye of an eagle flying
	 * over the terrain surface.
	 */
	bool eagle_mode(bool is_set = true);
private:
	void update_angles();
private:
	glm::vec3 cam_front,
			cam_right,
			cam_up;
	glm::mat4 cam_view;
	movable::mov_obj_ptr target_to_follow;
	target_following_options follow_opt;
	void evaluate_camera_options();
	camera_mode mode;
};

template<typename CUR_OPT,typename...OPT>
void my_camera::setup_following_options(CUR_OPT &&opt,
							OPT&&...remaining)
{
	switch( opt.first ) {
	case camera_opt::max_target_distance:
		LOG1("Setting up max camera distance to :", opt.second);
		follow_opt.target_max_distance = opt.second;
		break;
	case camera_opt::camera_pan:
		LOG1("Setting up camera pan to :", opt.second);
		follow_opt.camera_pan = opt.second;
		break;
	case camera_opt::camera_tilt:
		LOG1("Setting up camera tilt to:",opt.second);
		follow_opt.camera_tilt = opt.second;
		break;
	default:
		ERR("Unrecognized camera option: ",
			static_cast<int>(opt.first));
		break;
	}

	follow_opt.enabled_options.push_back( opt.first );

	if constexpr( sizeof...(remaining) != 0 ) {
			setup_following_options( remaining... );
	}
}

}

#endif //MY_CAMERA_HPP
