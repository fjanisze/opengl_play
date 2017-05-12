#ifndef MY_CAMERA_HPP
#define MY_CAMERA_HPP

#include "headers.hpp"
#include <movable_object.hpp>
#include <logger/logger.hpp>
#include <optional>
#include <types.hpp>

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

struct Camera_vectors
{
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 forward;

    Camera_vectors() = default;
};

class Camera : public scene::Movable
{
public:
    using pointer = std::shared_ptr< Camera >;
    Camera(glm::vec3 position,glm::vec3 target);
    void update_view_matrix();
    bool move(movement::direction direction, GLfloat amount) override;
    void modify_angle(movement::angle angle,GLfloat amount) override;
    glm::mat4 get_view();
    void set_position(const glm::vec3& pos) override;
    const Camera_vectors& get_vectors();
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
    Camera_vectors vectors;
    glm::mat4 view;
    camera_mode mode;
    /*
     * The angle between the camera
     * direction and X, required to perform
     * rotations around the target
     */
    GLfloat rotation_angle;
};

/*
 * Maintain the information about
 * the shape and size of the frustun
 */

struct Frustum_geometry
{
    GLfloat far_height;
    GLfloat far_width;
    GLfloat far_distance;
    GLfloat near_height;
    GLfloat near_width;
    GLfloat near_distance;

    /*
     * Frustum planes
     */
    types::point far_center;
    types::point far_top_left;
    types::point far_top_right;
    types::point far_bottom_left;
    types::point far_bottom_right;

    types::point near_center;
    types::point near_top_left;
    types::point near_top_right;
    types::point near_bottom_left;
    types::point near_bottom_right;

    Frustum_geometry() = default;
};

class Frustum
{
public:
    using pointer = std::shared_ptr< Frustum >;
    explicit Frustum( Camera::pointer cam ,
                      const GLfloat fov_angle,
                      const GLfloat aspect_ratio,
                      const GLfloat near_plane,
                      const GLfloat far_plane );
    /*
     * Update the frustum information and update
     * the geometric data
     */
    void update();
private:
    Camera::pointer camera;
    Frustum_geometry geometry;
    GLfloat fov;
    GLfloat ratio;
};

}

#endif //MY_CAMERA_HPP
