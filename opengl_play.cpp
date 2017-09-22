#include "opengl_play.hpp"
#include <iomanip>
#include <thread>
#include <chrono>
#include <core/units.hpp>
#include <core/maps.hpp>

namespace opengl_play {

namespace {
//Saved context for the GLFW callbacks
opengl_ui* ui_instance;
}

void mouse_click_callback( GLFWwindow* ctx,
                           int button,
                           int action,
                           int )
{
    ui_instance->ui_mouse_click( button, action );
}

void mouse_wheel_callback( GLFWwindow* ctx,
                           double x,
                           double y )
{
    ui_instance->ui_wheel_move( x, y );
}

void cursor_pos_callback( GLFWwindow* ctx,
                          double x,
                          double y )
{
    ui_instance->ui_mouse_move( x, y );
}

void mouse_cursor_enter_window( GLFWwindow* ctx,
                                int state )
{
    ui_instance->ui_mouse_enter_window( state );
}

void window_resize_callback( GLFWwindow* ctx,
                             int width,
                             int height )
{
    ui_instance->update_viewport( height, width );
}

void keyboard_press_callback( GLFWwindow* ctx,
                              int key,
                              int scode,
                              int action,
                              int )
{
    ui_instance->ui_keyboard_press( key, scode, action );
}


void opengl_ui::ui_mouse_click( GLint button, GLint action )
{
    if ( action != GLFW_PRESS ) {
        return;
    }
    if ( button == GLFW_MOUSE_BUTTON_LEFT ) {
        GLdouble x, y;
        glfwGetCursorPos( window_ctx, &x, &y );
        if ( key_status[ GLFW_KEY_LEFT_SHIFT ] != key_status_t::pressed ) {
            //    renderer->picking()->unpick();
        }
        renderer->picking()->pick_toggle( x, win_h - y );
    } else if ( button == GLFW_MOUSE_BUTTON_RIGHT ) {
        /*
         * If there's a selected unit then move that unit
         * otherwise create a new one.
         * Do not support movement of multiple objects
         */
        auto selected = renderer->picking()->get_selected_ids();
        auto pointed_model = renderer->picking()->get_pointed_model();
        if ( nullptr == pointed_model ) {
            //Nothing to do here.
            return;
        }
        auto lot = game_terrain->find_lot( pointed_model );
        if ( lot == nullptr ) {
            //The target must be always a terrain.
            return;
        }
        if ( selected.size() > 0 ) {
            //Try to move the unit
            if ( false == units->movements().multiple_move( selected,
                    lot ) ) {
                //Unpick everything..
                renderer->picking()->unpick();
            }
        } else {
            //Nothing selected, this must be a creation attempt
            auto new_unit = game_units->create( unit_id );
            auto target_lot = game_map->get_lot( lot->id );
            game_units->place( new_unit, target_lot );
        }
    }
}

void opengl_ui::ui_mouse_move( GLdouble x, GLdouble y )
{
    mouse_x_pos = x;
    mouse_y_pos = y;
    movement_processor.mouse_input( x, win_h - y );
    renderer->picking()->set_pointed_model( x, win_h - y );
    auto pointed_model = renderer->picking()->get_pointed_model();
}

void opengl_ui::ui_mouse_enter_window( int state )
{
    if ( GLFW_TRUE == state ) {
        LOG2( "Cursor enter the window!" );
    } else {
        LOG2( "Cursor exit the window!" );
    }
}

void opengl_ui::ui_wheel_move( GLdouble x, GLdouble y )
{
    movement_processor.mouse_input( x, y,
                                    graphic_scene::mouse_input_type::mouse_wheel );
}

void opengl_ui::ui_keyboard_press( GLint button,
                                   GLint scode,
                                   GLint action )
{
    if ( button < 0 ) {
        //Not recognized button
        return;
    }
    if ( action == GLFW_PRESS || action == GLFW_REPEAT ) {
        if ( button == GLFW_KEY_ESCAPE ) {
            glfwSetWindowShouldClose( window_ctx, 1 );
        } else {
            key_status[ button ] = key_status_t::pressed;
        }
        if ( button ==  GLFW_KEY_SPACE ) {
            renderer->picking()->unpick();
        }
    } else if ( action == GLFW_RELEASE ) {
        key_status[ button ] = key_status_t::not_pressed;
    }
    movement_processor.keyboard_input( button, scode, action );
}

void opengl_ui::evaluate_key_status()
{
}

void opengl_ui::setup_callbacks()
{

    glfwSetMouseButtonCallback( window_ctx,
                                mouse_click_callback );
    glfwSetScrollCallback( window_ctx,
                           mouse_wheel_callback );
    glfwSetCursorPosCallback( window_ctx,
                              cursor_pos_callback );
    glfwSetCursorEnterCallback( window_ctx,
                                mouse_cursor_enter_window );
    glfwSetWindowSizeCallback( window_ctx,
                               window_resize_callback );
    glfwSetKeyCallback( window_ctx,
                        keyboard_press_callback );
}

void opengl_ui::get_current_ctx_viewport()
{
    glfwGetFramebufferSize( window_ctx,
                            &win_w,
                            &win_h );
    glViewport( 0, 0,
                win_w,
                win_h );

    viewport = glm::vec4( 0.0, 0.0,
                          win_w, win_h );
}

void opengl_ui::init_text()
{
    info_string = factory< text_renderer::Renderable_text >::create();
    info_string->set_position( glm::vec3( 10.0, 10.0, 0.0 ) );
    info_string->set_color( glm::vec4( 1.0, 1.0, 1.0, 1.0 ) );
    info_string->set_scale( 0.7f );
    info_string->set_text( "0 fps" );
    info_string->rendering_state.set_enable();
    info_string->view_configuration.camera_space();
    renderer->add_renderable( info_string );
}

opengl_ui::opengl_ui( int win_width,
                      int win_heigth ) :
    win_h{ win_heigth },
    win_w{ win_width }
{
    LOG1( "Creating window, ", win_width, "/", win_heigth );
    //Setup GLFW
    if ( glfwInit() != GLFW_TRUE ) {
        ERR( "Unable to initialize GLFW" );
        throw std::runtime_error( "GLFW init failed" );
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    //Create the window
    window_ctx = glfwCreateWindow( win_width,
                                   win_heigth,
                                   "opengl play",
                                   nullptr,
                                   nullptr );
    if ( window_ctx == nullptr ) {
        ERR( "Unable to create the window!" );
        throw std::runtime_error( "Window creation failed" );
    }

    glfwMakeContextCurrent( window_ctx );

    //Init GLEW
    glewExperimental = GL_TRUE;
    GLenum glew_init_status = glewInit();
    if ( glew_init_status != GLEW_OK ) {
        ERR( "Unable to initialize GLEW: ",
             glewGetErrorString( glew_init_status ) );
        throw std::runtime_error( "GLEW Init failed!" );
    }

    camera = factory< graphic_scene::Camera >::create(
                 glm::vec3( 4.0, 4.0, 8 ),
                 glm::vec3( 0.0, 0.0, 0.0 )
             );
    camera->eagle_mode();

    for ( auto& elem : key_status ) {
        elem = key_status_t::not_pressed;
    }

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_DEPTH_TEST );

    //Enable the mouse cursor
    cursor = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    glfwSetCursor( window_ctx, cursor );
    glfwSetInputMode( window_ctx, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

    projection = glm::perspective( glm::radians( 45.0f ),
                                   ( GLfloat )win_w / ( GLfloat )win_h,
                                   1.0f, 100.0f );

    glm::mat4 def_ortho = glm::ortho( 0.0,
                                      ( double )win_w,
                                      0.0,
                                      ( double )win_h,
                                      0.0, 100.0 );

    renderer = factory< renderer::Core_renderer >::create(
                   types::win_size( win_w, win_h ),
                   projection,
                   def_ortho,
                   camera );

    init_text();

    /*
     * Enable face culling to avoid rendering
     * faces which are hidden behind other faces
     * from the camera view perspective.
     */
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glFrontFace( GL_CW );

    /*
     * Disable vsync, which in turns disable the
     * 60fps limit.
     */
    //glfwSwapInterval(0);
}

void opengl_ui::prepare_for_main_loop()
{
    LOG3( "Preparing the remaining environment." );
    //Save the instance pointer
    ui_instance = this;
    //Init the callbacks
    setup_callbacks();

    get_current_ctx_viewport();
}

void opengl_ui::setup_scene()
{
    //Register the camera as movable object
    graphic_scene::key_mapping_vec camera_keys = {
        { GLFW_KEY_W, { graphic_scene::movement::direction::top, { 0.3 } } },
        { GLFW_KEY_S, { graphic_scene::movement::direction::down, { 0.3 } } },
        { GLFW_KEY_A, { graphic_scene::movement::direction::left, { 0.3 } } },
        { GLFW_KEY_D, { graphic_scene::movement::direction::right, { 0.3 } } },
        { GLFW_KEY_Q, { graphic_scene::movement::direction::rotate_left, { 2.0 } } },
        { GLFW_KEY_E, { graphic_scene::movement::direction::rotate_right, { 2.0 } } },
    };


    graphic_scene::mouse_mapping_vec camera_mouse = {
        { graphic_scene::mouse_movement_types::wheel_up, { graphic_scene::movement::direction::forward, { 0.05 } } },
        { graphic_scene::mouse_movement_types::wheel_down, { graphic_scene::movement::direction::backward, { 0.05 } } },
    };

    movement_processor.register_movable_object( camera, camera_keys );
    movement_processor.register_movable_object( camera, camera_mouse );

    game_terrain = factory< graphic_terrains::Terrains >::create(
                       renderer::Core_renderer_proxy( renderer ) );

    core_maps::Maps maps_mgr( game_terrain );
    game_map = maps_mgr.create_random_map( 8 );

    units = factory< graphic_units::Units >::create(
                renderer::Core_renderer_proxy( renderer ) );
    game_units = factory< core_units::Units >::create( units, game_map );

    auto list_of_units = units->buildable_units();
    unit_id = list_of_units.front().def.id;

    light_1 = lighting::Light_factory<lighting::directional_light>::create(
                  glm::vec3( 30, 30, 30 ),
                  glm::vec4( 0.9, 0.8, 0.7, 1.0 ),
                  30 );

    light_2 = lighting::Light_factory<lighting::directional_light>::create(
                  glm::vec3( -10, 100, -10 ),
                  glm::vec4( 0.8, 0.7, 0.7, 1.0 ),
                  12 );

    renderer->scene_lights()->add_light( light_1 );
    renderer->scene_lights()->add_light( light_2 );
}


void opengl_ui::enter_main_loop()
{

    setup_scene();
    auto ref_time = std::chrono::system_clock::now();
    int  current_fps = 0;

    LOG2( "Entering main loop!" );
    std::string current_fps_string = "0 fps";
    long num_of_rendering_cycles{ 0 };
    while ( !glfwWindowShouldClose( window_ctx ) ) {
        ++current_fps;
        glfwPollEvents();
        evaluate_key_status();
        movement_processor.process_movements();
        units->movements().process_movements();

        auto current_time = std::chrono::system_clock::now();
        if ( std::chrono::duration_cast <
                std::chrono::milliseconds > (
                    current_time - ref_time ).count() > 1000 ) {
            ref_time = current_time;
            current_fps_string = ( std::to_string( current_fps ) + " fps" );
            current_fps = 0;
        }

        renderer->clear();
        num_of_rendering_cycles = renderer->render();

        auto yaw = camera->get_yaw(),
             pitch = camera->get_pitch(),
             roll = camera->get_roll();
        glm::vec3 pos = camera->get_position();
        auto pointed = renderer->picking()->get_pointed_model();
        types::id_type pointed_id = 0;
        if ( pointed != nullptr ) {
            pointed_id = pointed->id;
        }

        std::stringstream ss;
        ss << current_fps_string << " - " << std::setprecision( 2 ) << std::fixed << "yaw:" << yaw << ", pitch:" << pitch << ", roll:" << roll
           << ". x:" << pos.x << ",y:" << pos.y << ",z:" << pos.z << ", rendr cycles:"
           << num_of_rendering_cycles << ", Sel: " << pointed_id;

        /*
         * If anything was selected, remove the selection
         * and see if it's till valid in this round of
         * rendering
         */
        game_units->unselect();
        auto selected = renderer->picking()->get_selected_ids();
        if ( 1 == selected.size() ) {
            //Trigger the unit selection, if possible
            game_units->select( selected[ 0 ] );
        }

        info_string->set_text( ss.str() );

        glfwSwapBuffers( window_ctx );
    }
}

GLFWwindow* opengl_ui::get_win_ctx()
{
    return window_ctx;
}

void opengl_ui::update_viewport( int new_win_h,
                                 int new_win_w )
{
    win_h = new_win_h;
    win_w = new_win_w;

    glViewport( 0, 0,
                win_w,
                win_h );

    viewport = glm::vec4( 0.0, 0.0,
                          win_w, win_h );
}

opengl_ui::~opengl_ui()
{
    if ( nullptr != window_ctx ) {
        glfwTerminate();
    }
    if ( nullptr != cursor ) {
        glfwDestroyCursor( cursor );
    }
}

/*
 * Calculate and return the vector the a ray
 * casted from x,y.
 */
types::ray_t opengl_ui::ray_cast( GLdouble x, GLdouble y )
{
    glm::vec3 ms( x, win_h - y, 0 );
    auto cam_view = camera->get_view();

    glm::vec3 src = glm::unProject(
                        ms,
                        cam_view,
                        projection,
                        viewport );

    ms.z = 1;

    glm::vec3 dst = glm::unProject(
                        ms,
                        cam_view,
                        projection,
                        viewport );

    return { src, glm::normalize( dst - src ) };
}

glm::vec2 opengl_ui::ray_z_hit_point( const types::ray_t& ray,
                                      const GLfloat z_value )
{
    glm::vec3 target = ray.first;
    GLfloat l = 0,
            r = 1024; //Hopefully is big enough!
    while ( l < r ) {
        GLfloat mid = ( r + l ) / 2;
        target = ray.first + ray.second * mid;
        if ( glm::abs( target.z - z_value ) <= 0.00001 ) {
            //Looks like 0 :)
            break;
        }
        if ( target.z > 0 ) {
            l = mid + 0.0001;
        } else {
            r = mid - 0.0001;
        }
    }
    return glm::vec2( target.x, target.y );
}



}

int main()
{
    log_inst.set_thread_name( "MAIN" );
    log_inst.set_logging_level( logging::severity_type::debug0 );

    opengl_play::opengl_ui entry( 1920, 1280 );
    entry.prepare_for_main_loop();

    entry.enter_main_loop();
}
