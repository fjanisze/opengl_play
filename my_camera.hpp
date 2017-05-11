#ifndef MY_CAMERA_HPP
#define MY_CAMERA_HPP

#include "headers.hpp"
#include <movable_object.hpp>
#include <logger/logger.hpp>
#include <optional>

namespace scene
{

using scene::movement::angle;
using scene::movement::direction;

/*
 * Available camera working modes
 */
enum class camera_mode {
    space_mode, //Angles and vectors updated according to position and orientation
    eagle_mode  //The camera will not update the UP vector, forcing movements parallel to x/z
};


class Camera : public scene::Movable
{
public:
    using pointer = std::shared_ptr< Camera >;
    Camera(glm::vec3 position,glm::vec3 target);
    void update_cam_view();
    bool move(movement::direction direction, GLfloat amount) override;
    void modify_angle(movement::angle angle,GLfloat amount) override;
    glm::mat4 get_view();
    void set_position(const glm::vec3& pos) override;
    void set_target(scene::Movable::pointer object);

    glm::vec3 get_camera_front();
    /*
     * When set the camera will maintain a fixed
     * UP vector, in this way the camera
     * will emulate the eye of an eagle flying
     * over the terrain surface.
     */
    bool eagle_mode(bool is_set = true);
    void rotate_around(GLfloat amount) override;
private:
    void update_angles();
private:
    glm::vec3 cam_front;
    glm::vec3 cam_right;
    glm::vec3 cam_up;
    glm::vec3 cam_forward;
    glm::mat4 cam_view;
    camera_mode mode;
    /*
     * The angle between the camera
     * direction and X, required to perform
     * rotations around the target
     */
    GLfloat rotation_angle;
};

}

#endif //MY_CAMERA_HPP
