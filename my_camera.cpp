#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace scene
{

Camera::Camera(glm::vec3 position, glm::vec3 target) :
    mode{ camera_mode::space_mode }
{
    vectors.front = glm::normalize( target - position );
    set_position( position );
    update_angles();

    vectors.up = glm::vec3( 0.0, 0.0, cos( glm::degrees( current_pitch ) ) );//Point upward
    vectors.right = glm::normalize( glm::cross( vectors.front, vectors.up) );

    update_view_matrix();
    /*
     * Let's move foward always in the direction
     * of the map terrain. vectors.front will change
     * the attitude of the camera
     */
    vectors.forward = glm::vec3(0.0,0.0,-1.0);

    //Will be initialized when needed.
    rotation_angle = -1;
}


bool Camera::eagle_mode(bool is_set)
{
    bool old = ( mode == camera_mode::eagle_mode );
    if( is_set ) {
        /*
         * top/down and left/right will move
         * over Y and X
         */
        update_angles();
        mode = camera_mode::eagle_mode;
        vectors.up = glm::vec3(
                    sin( glm::radians( current_yaw ) ), //x
                    (current_position.y < 0 ? 1 : +1 ) * cos( glm::radians( current_yaw ) ), //y
                    0.0); //z

        vectors.right = glm::normalize( glm::cross( vectors.front,
                                                glm::vec3(0.0,0.0,1.0)) );
        vectors.right.z = 0;
        update_view_matrix();
    } else {
        mode = camera_mode::space_mode;
    }
    return old;
}

void Camera::rotate_around(GLfloat amount)
{
    if( amount == 0 ) {
        WARN1("rotate_around with argument 0 make no sense!");
        return;
    }
    /*
     * Calculate the target position, it is
     * the point at Z=0 in the direction where
     * the camera is looking at: vectors.front
     * TODO: Improve..
     */
    glm::vec3 cam_pos = get_position();
    glm::vec3 target = cam_pos;
    GLfloat l = 0,
            r = 1024; //Hopefully is big enough!
    while( l < r ) {
        GLfloat mid = ( r + l ) / 2;
        target = cam_pos + vectors.front * mid;
        if( glm::abs( target.z ) <= 0.00001 ) {
            //Looks like 0 :)
            break;
        }
        if( target.z > 0 ) {
            l = mid + 0.0001;
        } else {
            r = mid - 0.0001;
        }
    }
    //make sure it's zero and not some very small value :)
    target.z = 0.0f;
    //When rotating the distance do not change
    GLfloat distance = glm::distance( glm::vec3(cam_pos.x,cam_pos.y,0.0),
                                      target );

    //Eventually set the initial rotation angle
    if( rotation_angle < 0 ) {
        glm::vec3 deltas = glm::vec3(cam_pos.x,cam_pos.y,cam_pos.z) - target;
        rotation_angle = glm::degrees( std::acos( deltas.x / distance ) );
        if( cam_pos.y < 0 ) {
            rotation_angle = 360 - rotation_angle;
        }
        LOG3("Initial value of the rotation angle ",
             rotation_angle);
    }
    //Make sure that 0<rotation_angle<360
    rotation_angle += amount;
    if( rotation_angle >= 359.99 ) {
        rotation_angle = 0.01;
    }
    if( rotation_angle <= 0 ) {
        rotation_angle = 359.99;
    }
    /*
     * Calculate the new position on the base
     * of the amount of rotation
     */
    GLfloat angle = glm::radians( rotation_angle );

    glm::vec3 new_pos(
                target.x + cos( angle ) * distance,
                target.y + sin( angle ) * distance,
                cam_pos.z
                );
    /*
     * Update camera vectors to make sure
     * we point in the right direction
    */
    vectors.front = glm::normalize( target - new_pos );

    vectors.up = vectors.front;
    vectors.up.z = 0;

    vectors.right = glm::normalize( glm::cross( vectors.front,
                                            glm::vec3( 0.0, 0.0, 1.0 ) ) );

    set_position( new_pos );
    update_angles();
}

void Camera::update_view_matrix()
{
    view = glm::lookAt( current_position, current_position + vectors.front, vectors.up );
}

bool Camera::move(movement::direction direction, GLfloat amount)
{
    bool ret = true;
    switch(direction) {
    case movement::direction::right:
        current_position += vectors.right * amount;
        break;
    case movement::direction::left:
        current_position -= vectors.right * amount;
        break;
    case movement::direction::top:
        current_position +=  vectors.up * amount;
        break;
    case movement::direction::down:
        current_position -= vectors.up * amount;
        break;
    case movement::direction::forward:
        if( current_position.z > 2 ) {
            current_position += vectors.forward * amount;
            modify_angle( movement::angle::pitch, 0.15 );
        }
        break;
    case movement::direction::backward:
        if( current_position.z < 30 ) {
            current_position -= vectors.forward * amount;
            modify_angle( movement::angle::pitch, -0.15 );
        }
        break;
    default:
        ERR("my_camera::move: Unknow direction, ",
            static_cast<int>(direction));
        ret = false;
        break;
    }
    if( ret ) {
        update_view_matrix();
    }
    return ret;
}

void Camera::modify_angle(movement::angle angle,GLfloat amount)
{
    switch( angle ) {
    case movement::angle::pitch:
        current_pitch += amount;
        break;
    case movement::angle::yaw:
        current_yaw += amount;
        break;
    case movement::angle::roll:
        current_roll += amount;
        break;
    default:
        ERR("my_camera::modify_angle: Invalid angle, type:",
            static_cast<int>(angle));
        return;
    }

    if( current_yaw >= 359.99 ) current_yaw = 0.01;
    if( current_yaw <= 0.0 ) current_yaw = 359.99;
    if( current_roll >= 359.99 ) current_roll = 0.01;
    if( current_roll <= 0.0 ) current_roll = 359.99;
    if( current_pitch >= 89.99 ) current_pitch = 89.99;
    if( current_pitch <= -89.99 ) current_pitch = -89.99;

    vectors.front.x += sin( glm::radians( current_yaw ) ) * cos( glm::radians( current_pitch ) );
    vectors.front.y += cos( glm::radians( current_yaw ) ) * cos( glm::radians( current_pitch ) );
    vectors.front.z += sin( glm::radians( current_pitch ) );

    vectors.front = glm::normalize( vectors.front );

    vectors.right = glm::normalize( glm::cross( vectors.front, vectors.up ) );

    if( mode == camera_mode::space_mode ) {
        vectors.up = glm::normalize( glm::cross( vectors.right, vectors.front ) );
    }

    update_view_matrix();
}

/*
 * Those Euler coordinates can be extracted
 * from the current direction vector
 */
void Camera::update_angles()
{
    current_pitch = glm::degrees( asin( vectors.front.z ) );
    current_yaw = glm::degrees( atan2( vectors.front.x, vectors.front.y) );
    current_yaw = current_yaw > 0 ? current_yaw : 360 + current_yaw;
}

glm::mat4 Camera::get_view()
{
    return view;
}

void Camera::set_position(const glm::vec3& pos)
{
    current_position = pos;
    update_view_matrix();
}

const Camera_vectors &Camera::get_vectors()
{
    return vectors;
}

glm::vec3 Camera::get_camera_front()
{
    return vectors.front;
}

//////////////////////////////////////
/// Frustum implementation
/////////////////////////////////////

Frustum::Frustum(Camera::pointer cam,
                 const GLfloat fov_angle,
                 const GLfloat aspect_ratio,
                 const GLfloat near_plane,
                 const GLfloat far_plane) :
    camera{ cam },
    fov{ glm::radians( fov_angle ) },
    ratio{ aspect_ratio }
{
    LOG3("Creating a new Frustum object!");
    geometry.far_distance = far_plane;
    geometry.near_distance = near_plane;
}

void Frustum::update()
{
    //Height / Width calculations
    const GLfloat sin_half_fov = glm::sin( fov / 2 );
    const GLfloat cos_half_fov = glm::cos( fov / 2 );
    geometry.far_height = 2 * sin_half_fov * geometry.far_distance / cos_half_fov;
    geometry.far_width = geometry.far_height * ratio;
    geometry.near_height = 2 * sin_half_fov * geometry.near_distance / cos_half_fov;
    geometry.near_width = geometry.near_height * ratio;

    auto cam_vecs = camera->get_vectors();
    const types::point cam_pos = camera->get_position();

    //Calculate the relevant frustum points and planes

    //FAR plane
    const types::point far_val_h{ cam_vecs.up * geometry.far_height / 2.0f };
    const types::point far_val_w{ cam_vecs.right * geometry.far_width / 2.0f };
    geometry.far_center = cam_pos + cam_vecs.front * geometry.far_distance;
    geometry.far_top_left = geometry.far_center + far_val_h - far_val_w;
    geometry.far_top_right = geometry.far_center + far_val_h + far_val_w;
    geometry.far_bottom_left = geometry.far_center - far_val_h - far_val_w;
    geometry.far_bottom_right = geometry.far_center - far_val_h + far_val_w;

    //NEAR plane
    const types::point near_val_h{ cam_vecs.up * geometry.near_height / 2.0f };
    const types::point near_val_w{ cam_vecs.right * geometry.near_width / 2.0f };
    geometry.near_center = cam_pos + cam_vecs.front * geometry.near_distance;
    geometry.near_top_left = geometry.near_center + near_val_h - near_val_w;
    geometry.near_top_right = geometry.near_center + near_val_h + near_val_w;
    geometry.near_bottom_left = geometry.near_center - near_val_h - near_val_w;
    geometry.near_bottom_right = geometry.near_center - near_val_h + near_val_w;
}

}
