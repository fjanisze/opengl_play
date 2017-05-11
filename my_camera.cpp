#include "my_camera.hpp"
#include <logger/logger.hpp>

namespace scene
{

Camera::Camera(glm::vec3 position, glm::vec3 target) :
    cam_front( glm::normalize( target - position ) ),
    mode{ camera_mode::space_mode }
{
    set_position( position );
    update_angles();

    cam_up = glm::vec3( 0.0, 0.0, cos( glm::degrees( current_pitch ) ) );//Point upward
    cam_right = glm::normalize( glm::cross( cam_front, cam_up) );

    update_cam_view();
    /*
     * Let's move foward always in the direction
     * of the map terrain. cam_front will change
     * the attitude of the camera
     */
    cam_forward = glm::vec3(0.0,0.0,-1.0);

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
        cam_up = glm::vec3(
                    sin( glm::radians( current_yaw ) ), //x
                    (current_position.y < 0 ? 1 : +1 ) * cos( glm::radians( current_yaw ) ), //y
                    0.0); //z

        cam_right = glm::normalize( glm::cross( cam_front,
                                                glm::vec3(0.0,0.0,1.0)) );
        cam_right.z = 0;
        update_cam_view();
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
     * the camera is looking at: cam_front
     * TODO: Improve..
     */
    glm::vec3 cam_pos = get_position();
    glm::vec3 target = cam_pos;
    GLfloat l = 0,
            r = 1024; //Hopefully is big enough!
    while( l < r ) {
        GLfloat mid = ( r + l ) / 2;
        target = cam_pos + cam_front * mid;
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
    cam_front = glm::normalize( target - new_pos );

    cam_up = cam_front;
    cam_up.z = 0;

    cam_right = glm::normalize( glm::cross( cam_front,
                                            glm::vec3( 0.0, 0.0, 1.0 ) ) );

    set_position( new_pos );
    update_angles();
}

void Camera::update_cam_view()
{
    cam_view = glm::lookAt( current_position, current_position + cam_front, cam_up );
}

bool Camera::move(mov_direction direction, GLfloat amount)
{
    bool ret = true;
    switch(direction) {
    case mov_direction::right:
        current_position += cam_right * amount;
        break;
    case mov_direction::left:
        current_position -= cam_right * amount;
        break;
    case mov_direction::top:
        current_position +=  cam_up * amount;
        break;
    case mov_direction::down:
        current_position -= cam_up * amount;
        break;
    case mov_direction::forward:
        if( current_position.z > 2 ) {
            current_position += cam_forward * amount;
            modify_angle( mov_angles::pitch, 0.15 );
        }
        break;
    case mov_direction::backward:
        if( current_position.z < 30 ) {
            current_position -= cam_forward * amount;
            modify_angle( mov_angles::pitch, -0.15 );
        }
        break;
    default:
        ERR("my_camera::move: Unknow direction, ",
            static_cast<int>(direction));
        ret = false;
        break;
    }
    if( ret ) {
        update_cam_view();
    }
    return ret;
}

void Camera::modify_angle(mov_angles angle,GLfloat amount)
{
    switch( angle ) {
    case mov_angles::pitch:
        current_pitch += amount;
        break;
    case mov_angles::yaw:
        current_yaw += amount;
        break;
    case mov_angles::roll:
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

    cam_front.x += sin( glm::radians( current_yaw ) ) * cos( glm::radians( current_pitch ) );
    cam_front.y += cos( glm::radians( current_yaw ) ) * cos( glm::radians( current_pitch ) );
    cam_front.z += sin( glm::radians( current_pitch ) );

    cam_front = glm::normalize( cam_front );

    cam_right = glm::normalize( glm::cross( cam_front, cam_up ) );

    if( mode == camera_mode::space_mode ) {
        cam_up = glm::normalize( glm::cross( cam_right, cam_front ) );
    }

    update_cam_view();
}

/*
 * Those Euler coordinates can be extracted
 * from the current direction vector
 */
void Camera::update_angles()
{
    current_pitch = glm::degrees( asin( cam_front.z ) );
    current_yaw = glm::degrees( atan2( cam_front.x, cam_front.y) );
    current_yaw = current_yaw > 0 ? current_yaw : 360 + current_yaw;
}

glm::mat4 Camera::get_view()
{
    return cam_view;
}

void Camera::set_position(const glm::vec3& pos)
{
    current_position = pos;
    update_cam_view();
}

glm::vec3 Camera::get_camera_front()
{
    return cam_front;
}

}
