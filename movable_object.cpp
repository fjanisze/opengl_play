#include "movable_object.hpp"
#include "logger/logger.hpp"
#include <algorithm>

namespace scene {

//////////////////////////////////////
/// movable_object implementation
/////////////////////////////////////

Movable::Movable() :
    model{ glm::mat4() },
    current_position{ glm::vec3() },
    current_yaw{ 0 },
    current_pitch{ 0 },
    current_roll{ /* 90 */ 0 },
    current_scale{ 1.0 }
{

}

void Movable::set_position( const glm::vec3& position )
{
    glm::vec3 translation_vector = position - current_position;
    model = glm::translate( model,
                            translation_vector );
    current_position = position;
}

glm::vec3 Movable::get_position()
{
    return current_position;
}

void Movable::set_yaw( GLfloat yaw )
{
    modify_angle( movement::angle::yaw, current_yaw - yaw );
    current_yaw = yaw;
}

void Movable::set_pitch( GLfloat pitch )
{
    modify_angle( movement::angle::pitch, current_pitch - pitch );
    current_pitch = pitch;
}

void Movable::set_roll( GLfloat roll )
{
    modify_angle( movement::angle::roll, current_roll - roll );
    current_roll = roll;
}

void Movable::set_scale( GLfloat scale )
{
    current_scale = scale;
}

GLfloat Movable::get_yaw()
{
    return current_yaw;
}

GLfloat Movable::get_pitch()
{
    return current_pitch;
}

GLfloat Movable::get_roll()
{
    return current_roll;
}

void Movable::modify_angle( movement::angle angle,
                            GLfloat amount )
{
    static glm::vec3 rotation_angles[3] = {
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0}
    };
    model = glm::rotate( model,
                         glm::radians( amount ),
                         rotation_angles[static_cast<int>( angle )] );
    switch ( angle ) {
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
        ERR( "modify_angle: Unknow angle!" );
    }
}


bool Movable::move( movement::direction direction,
                    GLfloat amount )
{
    if ( amount < 0 ) {
        ERR( "movable_object::move: amount must be >= 0" );
        return false;
    }
    glm::vec3 translation_vector;
    bool ret = true;

    switch ( direction ) {
    case movement::direction::right:
        amount *= -1;
    case movement::direction::left:
        translation_vector.x = amount;
        break;
    case movement::direction::down:
        amount *= -1;
    case movement::direction::top:
        translation_vector.y = amount;
        break;
    case movement::direction::backward:
        amount *= -1;
    case movement::direction::forward:
        translation_vector.z = amount;
        break;
    default:
        WARN2( "movable_object::move: Use modify_angle to change the angles!" );
        ret = false;
        break;
    }
    if ( true == ret ) {
        model = glm::translate( model,
                                translation_vector );
        current_position = glm::vec3( model[3].x, model[3].y, model[3].z );
    }
    return ret;
}

void Movable::rotate_around( GLfloat amount )
{
    ERR( "NOT IMPLEMENTED" );
}

movement_mapping& Movable::get_movement_setup()
{
    return movement_setup;
}

//////////////////////////////////////
/// object_movement_processor implementation
/////////////////////////////////////

Movement_processor::Movement_processor() :
    last_mouse_x_position{ -1 },
    last_mouse_y_position{ -1 }
{
    LOG1( "Creating object_movement_processor" );
    key_status.resize( 1024 );
    std::fill( key_status.begin(), key_status.end(), key_status_t::not_pressed );
}

void Movement_processor::mouse_input( GLdouble new_x,
                                      GLdouble new_y,
                                      mouse_input_type type )
{
    if ( type == mouse_input_type::mouse_wheel ) {
        //Not supporting that much movements.
        if ( new_y > 0 ) {
            mouse_status[ mouse_movement_types::wheel_up ] = 60;
        } else {
            mouse_status[ mouse_movement_types::wheel_down ] = 60;
        }
    } else {
        static bool first_input{ true };
        if ( false == first_input ) {
            GLdouble x_delta = last_mouse_x_position - new_x,
                     y_delta = last_mouse_y_position - new_y;

            if ( x_delta > 0 ) {
                mouse_status[ mouse_movement_types::yaw_increase ] += x_delta;
            } else if ( x_delta < 0 ) {
                mouse_status[ mouse_movement_types::yaw_decrease ] += -x_delta;
            }

            if ( y_delta > 0 ) {
                mouse_status[ mouse_movement_types::pitch_increse ] += y_delta;
            } else if ( y_delta < 0 ) {
                mouse_status[ mouse_movement_types::pitch_decrease ] += -y_delta;
            }
        } else {
            first_input = false;
        }
        last_mouse_x_position = new_x;
        last_mouse_y_position = new_y;
    }
}

void Movement_processor::keyboard_input( int key,
        int scan_code,
        int action )
{
    while ( key >= key_status.size() ) {
        //This should rather not happen
        std::size_t cur_size = key_status.size();
        ERR( "key_status is too small: ",
             cur_size, ", increasing size!" );
        key_status.resize( cur_size * 2, key_status_t::not_pressed );
    }
    if ( action == GLFW_PRESS || action == GLFW_REPEAT ) {
        key_status[ key ] = key_status_t::pressed;
        process_speed_selectors( key );
    } else if ( action == GLFW_RELEASE ) {
        key_status[ key ] = key_status_t::not_pressed;
    }
}

void Movement_processor::process_movements()
{
    /*
     * Process keyboard movements
     */
    process_impl_keyboard();
    /*
     * Process mouse movements
     */
    process_impl_mouse();
    /*
     * Are there movement to perform
     * due to tracking setup?
     */
    object_tracking.process_tracking();
}

void Movement_processor::register_movable_object( Movable::pointer obj,
        const key_mapping_vec& key_mapping )
{
    movement_mapping& mov_setup = obj->get_movement_setup();
    for ( auto& key : key_mapping ) {
        mov_setup[ key.second.direction ] = key.second;
        keyb_mapping[ key.first ][ obj ].push_back( key.second.direction );
    }
}

void Movement_processor::register_movable_object( Movable::pointer obj,
        const mouse_mapping_vec& mapping )
{
    movement_mapping& mov_setup = obj->get_movement_setup();
    for ( auto& mouse_mov : mapping ) {
        mov_setup[ mouse_mov.second.direction ] = mouse_mov.second;
        mouse_mapping[ mouse_mov.first ][ obj ].push_back( mouse_mov.second.direction );
    }
}

void Movement_processor::unregister_movable_object( Movable::pointer obj )
{
    for ( auto& elem : keyb_mapping ) {
        elem.second.erase( obj );
    }
}

void Movement_processor::register_speed_selectors( Movable::pointer obj,
        const speed_selector& selector )
{
    for ( auto& elem : selector ) {
        speed_selectors[ elem.key ][ obj ].push_back( elem );
    }
}

/*
 * Verify whether the pressed key is not
 * a trigger for a speed selection. If that's the case,
 * the modify the current selector accordingly
 */
void Movement_processor::process_speed_selectors( key_code_t pressed_key )
{
    /*
     * Anybody registered a selector for this key?
     */
    auto it = speed_selectors.find( pressed_key );
    if ( it != speed_selectors.end() ) {
        /*
         * Ok, pick the selector for each object
         * which is registered to trigger speed changes
         * for this key
         */
        for ( auto& objects : it->second ) {
            Movable::pointer obj = objects.first;
            movement_mapping& mov_setup = obj->get_movement_setup();
            /*
             * Any speed selector corresponding to the
             * key 'pressed_key'?
             */
            for ( auto& sel : objects.second ) {
                if ( sel.key == pressed_key ) {
                    //Ok, trigger the speed change
                    if ( mov_setup[ sel.direction ].speed.size() < sel.idx ) {
                        ERR( "Unable to change speed, selector with idx: ", sel.idx, " is invalid!" );
                    } else {
                        mov_setup[ sel.direction ].current_speed = sel.idx;
                    }
                }
            }
        }
    }
}


Tracking_processor& Movement_processor::tracking()
{
    return object_tracking;
}

void Movement_processor::process_impl_keyboard()
{
    for ( int key = 0 ; key < key_status.size() ; ++key ) {
        if ( key_status[ key ] == key_status_t::pressed ) {
            /*
             * the key 'key' is pressed, check whether anybody
             * has registered a movable_object to perform
             * a movement if this key is pressed.
             */
            auto it = keyb_mapping.find( key );
            if ( it != keyb_mapping.end() ) {
                /*
                 * For each registered object, perform the
                 * proper movement
                 */
                trigger_proper_movement( it->second, 1.0 );
            }
        }
    }
}

void Movement_processor::process_impl_mouse()
{
    for ( auto& movement : mouse_status ) {
        if ( movement.second != 0 ) {
            GLfloat amount = movement.second / 10;
            //Is anybody registered for this movement?
            if ( mouse_mapping[ movement.first ].empty() ) {
                movement.second = 0;//Nope? Clean..
            } else {
                trigger_proper_movement( mouse_mapping[ movement.first ], amount );

                movement.second = std::max<GLfloat>( movement.second - amount, 0.0 );
                /*
                 * The bigger is this constant (1.0)
                 * then more softly the object will terminate
                 * is movement.
                 */
                if ( movement.second < 1.0 ) {
                    movement.second = 0;
                }
            }
        }
    }
}

void Movement_processor::trigger_proper_movement( obj_dir_map& dir_map,
        GLfloat speed_amount )
{
    for ( auto& movable : dir_map ) {
        Movable::pointer obj = movable.first;
        movement_mapping& mov_setup = obj->get_movement_setup();
        for ( auto& dir : movable.second ) {
            auto sp_setup = mov_setup[ dir ];
            GLfloat speed = sp_setup.speed[ sp_setup.current_speed ] * speed_amount;
            switch ( dir ) {
            case movement::direction::left:
            case movement::direction::right:
            case movement::direction::top:
            case movement::direction::down:
            case movement::direction::forward:
            case movement::direction::backward:
                obj->move( dir, speed );
                break;
            case movement::direction::yaw_inc:
                speed *= -1;
            case movement::direction::yaw_dec:
                obj->modify_angle( movement::angle::yaw, speed );
                break;
            case movement::direction::pitch_dec:
                speed *= -1;
            case movement::direction::pitch_inc:
                obj->modify_angle( movement::angle::pitch, speed );
                break;
            case movement::direction::roll_dec:
                speed *= -1;
            case movement::direction::roll_inc:
                obj->modify_angle( movement::angle::roll, speed );
                break;
            case movement::direction::rotate_right:
                speed *= -1;
            case movement::direction::rotate_left:
                obj->rotate_around( speed );
                break;
            default:
                ERR( "trigger_proper_movement: Unrecognize movement direction" );
                break;
            }
        }
    }
}

//////////////////////////////////////
/// tracking_processor implementation
/////////////////////////////////////

bool Tracking_processor::new_tracking( Movable::pointer target,
                                       Movable::pointer object,
                                       GLfloat distance_threashold,
                                       bool smooth_tracking )
{
    LOG1( "new_tracking: New tracking setup!" );
    if ( tracking_data.find( object ) != tracking_data.end() ) {
        WARN1( "tracking_processor::new_tracking: Cannot add the same object twice!" );
        return false;
    }
    glm::vec3 target_pos = target->get_position(),
              object_pos = object->get_position();
    tracking_info new_track = {
        target,
        object,
        target_pos,
        object_pos,
        distance_threashold,
        smooth_tracking
    };
    auto ret = tracking_data.insert( { object, new_track } );
    return ret.second;
}

void Tracking_processor::process_tracking()
{
    for ( auto& entry : tracking_data ) {
        auto target_pos = entry.second.target->get_position();
        auto object_pos = entry.second.object->get_position();
        GLfloat current_distance = glm::distance( target_pos, object_pos );
        //We shall follow the target which is getting too far
        if ( current_distance > entry.second.distance_threshold ) {
            glm::vec3 dir_vector = target_pos - object_pos;
            if ( entry.second.smooth_tracking ) {
                dir_vector /= 100;
            }
            object_pos = object_pos + dir_vector;
            entry.second.object->set_position( object_pos );
        }
        entry.second.last_target_position = target_pos;
        entry.second.last_object_position = object_pos;
        entry.second.last_recorded_distance = current_distance;
    }
}

GLfloat Tracking_processor::get_dist_from_target( Movable::pointer object )
{
    auto it = tracking_data.find( object );
    if ( it != tracking_data.end() ) {
        return it->second.last_recorded_distance;
    }
    return -1;
}

}
