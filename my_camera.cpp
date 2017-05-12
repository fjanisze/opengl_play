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

    const glm::vec3 world_up = glm::vec3(0.0f,0.0f,1.0f);
    vectors.right = glm::normalize( glm::cross( vectors.front, world_up ) );
    vectors.up = glm::normalize( glm::cross( vectors.right, vectors.front ) );

    update_view_matrix();

    const GLfloat distance = glm::distance( target,
                                            glm::vec3( position.x,
                                                       position.y,
                                                       0.0f ) );
    rotation_angle = glm::degrees( glm::acos( position.x / distance ) );
    if( position.y < 0 ) {
        rotation_angle = 360 - rotation_angle;
    }

    LOG3("Initial value of the rotation angle ",
         rotation_angle);

    xy_plane.create( types::point(10.0f,10.0f,0.0f),
                     types::point(-10.0f,-10.0f,0.0f),
                     types::point(10.0f,-10.0f,0.0f) );
}

/*
 * When running in 'eagle mode' the Y axis
 * is perpendicular to the 'ground'. To move
 * toward the ground the front vector is used.
 */
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
        recalculate_vectors();

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
    const types::point cam_pos = get_position();
    const types::point target = xy_plane.intersection( vectors.front, cam_pos );

    //When rotating the distance do not change
    const GLfloat distance = glm::distance(
                glm::vec3(cam_pos.x,cam_pos.y,0.0f),
                target );

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

    const glm::vec3 new_pos(
                target.x + glm::cos( glm::radians( rotation_angle ) ) * distance,
                target.y + glm::sin( glm::radians( rotation_angle ) ) * distance,
                cam_pos.z
                );
    /*
     * Update camera vectors to make sure
     * we point in the right direction
    */
    vectors.front = glm::normalize( target - new_pos );
    recalculate_vectors();

    set_position( new_pos );
    update_angles();
    update_view_matrix();
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
            current_position += vectors.front * amount;
            modify_angle( movement::angle::pitch, 0.15 );
        }
        break;
    case movement::direction::backward:
        if( current_position.z < 30 ) {
            current_position -= vectors.front * amount;
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

    recalculate_vectors();

    update_view_matrix();
}

/*
 * Those Euler coordinates can be extracted
 * from the current direction vector
 */
void Camera::update_angles()
{
    current_pitch = glm::degrees( glm::asin( vectors.front.z ) );
    current_yaw = glm::degrees( glm::atan( vectors.front.x, vectors.front.y) );
    current_yaw = current_yaw > 0 ? current_yaw : 360 + current_yaw;
}

/*
 * Recalculate the up and right vector
 * on the base of the current front vector
 */
void Camera::recalculate_vectors()
{
    if( mode == camera_mode::eagle_mode ) {
        const types::point position = get_position();
        const types::point target = xy_plane.intersection( vectors.front, position );

        vectors.up = glm::normalize( glm::vec3( target.x,
                                                target.y,
                                                position.z ) - position );


        vectors.right = glm::normalize( glm::cross( vectors.front, vectors.up ) );
    } else {
        vectors.right = glm::normalize( glm::cross(
                                            vectors.front,
                                            glm::vec3(0.0f,0.0f,1.0f)
                                             ) );
        vectors.up = glm::normalize( glm::cross( vectors.right,
                                                 vectors.front ) );
    }
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
    const GLfloat tan_half_fov = glm::tan( fov / 2.0f );
    geometry.far_height = 2.0f * tan_half_fov * geometry.far_distance;
    geometry.far_width = geometry.far_height * ratio;
    geometry.near_height = 2.0f * tan_half_fov * geometry.near_distance;
    geometry.near_width = geometry.near_height * ratio;

    auto cam_vecs = camera->get_vectors();
    const types::point cam_pos = camera->get_position();

    //Calculate the relevant frustum points and planes

    /*
     * We need to calculate the 'real' up vector,
     * the one provided by the camera might have
     * been altered to permit specific camera movements
     * (see eagle mode)
     */
    const glm::vec3 up_vector = glm::cross( cam_vecs.right, cam_vecs.front );
    //FAR points
    const types::point far_val_h{ up_vector * geometry.far_height / 2.0f };
    const types::point far_val_w{ cam_vecs.right * geometry.far_width / 2.0f };
    geometry.far_center = cam_pos + cam_vecs.front * geometry.far_distance;
    geometry.far_top_left = geometry.far_center + far_val_h - far_val_w;
    geometry.far_top_right = geometry.far_center + far_val_h + far_val_w;
    geometry.far_bottom_left = geometry.far_center - far_val_h - far_val_w;
    geometry.far_bottom_right = geometry.far_center - far_val_h + far_val_w;

    //NEAR points
    const types::point near_val_h{ up_vector * geometry.near_height / 2.0f };
    const types::point near_val_w{ cam_vecs.right * geometry.near_width / 2.0f };
    geometry.near_center = cam_pos + cam_vecs.front * geometry.near_distance;
    geometry.near_top_left = geometry.near_center + near_val_h - near_val_w;
    geometry.near_top_right = geometry.near_center + near_val_h + near_val_w;
    geometry.near_bottom_left = geometry.near_center - near_val_h - near_val_w;
    geometry.near_bottom_right = geometry.near_center - near_val_h + near_val_w;

    //Planes
    //TOP
    geometry.planes[ 0 ].create( geometry.near_top_right,
                                 geometry.near_top_left,
                                 geometry.far_top_right );
    //BOTTOM
    geometry.planes[ 1 ].create( geometry.near_bottom_left,
                                 geometry.near_bottom_right,
                                 geometry.far_bottom_right );
    //LEFT
    geometry.planes[ 2 ].create( geometry.near_top_left,
                                 geometry.near_bottom_left,
                                 geometry.far_bottom_left );
    //RIGHT
    geometry.planes[ 3 ].create( geometry.near_bottom_right,
                                 geometry.near_top_right,
                                 geometry.far_bottom_right );
    //NEAR PLANE
    geometry.planes[ 4 ].create( geometry.near_top_left,
                                 geometry.near_top_right,
                                 geometry.near_bottom_right );

    //FAR PLANE
    geometry.planes[ 5 ].create( geometry.far_top_right,
                                 geometry.far_top_left,
                                 geometry.far_bottom_left );
}

GLfloat Frustum::is_inside( const point &pt ) const
{
    GLfloat min_dist{ 0 };
    for( int i{ 0 } ; i < 6 ; ++i ) {
        const GLfloat dist = geometry.planes[ i ].distance( pt );
        if( dist < 0 ) {
            return dist;
        } else {
            min_dist = glm::min( min_dist, dist );
        }
    }
    return min_dist;
}

void Plane::create( const types::point p0,
                    const types::point p1,
                    const types::point p2 )
{
    const types::vector n = glm::normalize( glm::cross( p1 - p0, p2 - p0 ) );
    const GLfloat coef_d = -glm::dot( n, p0 );
    coefs = glm::vec4( n, coef_d );
}

types::point Plane::intersection(
        const glm::vec3 &direction,
        const point &position ) const
{
    const GLfloat t_value = - //Mind the -
                      ( coefs.x * position.x +
                        coefs.y * position.y +
                        coefs.z * position.z +
                        coefs.w )
                        /
                       ( coefs.x * direction.x +
                         coefs.y * direction.y +
                         coefs.z * direction.z );
    return types::point(
                position.x + direction.x * t_value,
                position.y + direction.y * t_value,
                position.z + direction.z * t_value
                );
}

GLfloat Plane::distance(const point &pt) const
{
    return coefs.x * pt.x + coefs.y * pt.y +
           coefs.z * pt.z + coefs.w;
}

}
