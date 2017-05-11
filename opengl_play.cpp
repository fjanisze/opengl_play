#include "opengl_play.hpp"
#include <iomanip>
#include <thread>
#include <chrono>

namespace opengl_play
{

namespace
{
//Saved context for the GLFW callbacks
opengl_ui* ui_instance;
}

void mouse_click_callback(GLFWwindow *ctx,
                          int button,
                          int action,
                          int)
{
    ui_instance->ui_mouse_click(button,action);
}

void mouse_wheel_callback(GLFWwindow* ctx,
                          double x,
                          double y)
{
    ui_instance->ui_wheel_move(x,y);
}

void cursor_pos_callback(GLFWwindow *ctx,
                         double x,
                         double y)
{
    ui_instance->ui_mouse_move(x,y);
}

void mouse_cursor_enter_window(GLFWwindow* ctx,
                               int state)
{
    ui_instance->ui_mouse_enter_window(state);
}

void window_resize_callback(GLFWwindow *ctx,
                            int width,
                            int height)
{
    ui_instance->update_viewport(height,width);
}

void keyboard_press_callback(GLFWwindow* ctx,
                             int key,
                             int scode,
                             int action,
                             int)
{
    ui_instance->ui_keyboard_press(key,scode,action);
}


void opengl_ui::ui_mouse_click(GLint button, GLint action)
{
    if( button == GLFW_MOUSE_BUTTON_LEFT &&
        action == GLFW_PRESS ) {
    }
}

void opengl_ui::ui_mouse_move(GLdouble x, GLdouble y)
{
    mouse_x_pos = x;
    mouse_y_pos = y;
    movement_processor.mouse_input(x, win_h - y);
    auto it = renderer->select_model( x, win_h - y );
}

void opengl_ui::ui_mouse_enter_window(int state)
{
    if( GLFW_TRUE == state ) {
        LOG2("Cursor enter the window!");
    } else {
        LOG2("Cursor exit the window!");
    }
}

void opengl_ui::ui_wheel_move(GLdouble x, GLdouble y)
{
    movement_processor.mouse_input(x, y,
                                   movable::mouse_input_type::mouse_wheel);
}

void opengl_ui::ui_keyboard_press(GLint button,
                                  GLint scode,
                                  GLint action)
{
    if( button < 0 ) {
        //Not recognized button
        return;
    }
    if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
        if( button == GLFW_KEY_ESCAPE ) {
            glfwSetWindowShouldClose(window_ctx,1);
        } else {
            key_status[ button ] = key_status_t::pressed;
        }
    } else if( action == GLFW_RELEASE ) {
        key_status[ button ] = key_status_t::not_pressed;
    }
    movement_processor.keyboard_input(button,scode,action);
}

void opengl_ui::evaluate_key_status()
{
}

void opengl_ui::setup_callbacks()
{

    glfwSetMouseButtonCallback(window_ctx,
                               mouse_click_callback);
    glfwSetScrollCallback(window_ctx,
                          mouse_wheel_callback);
    glfwSetCursorPosCallback(window_ctx,
                             cursor_pos_callback);
    glfwSetCursorEnterCallback(window_ctx,
                               mouse_cursor_enter_window);
    glfwSetWindowSizeCallback(window_ctx,
                              window_resize_callback);
    glfwSetKeyCallback(window_ctx,
                       keyboard_press_callback);
}

void opengl_ui::get_current_ctx_viewport()
{
    glfwGetFramebufferSize(window_ctx,
                           &win_w,
                           &win_h);
    glViewport(0,0,
               win_w,
               win_h);

    viewport = glm::vec4(0.0,0.0,
                         win_w,win_h);
}

void opengl_ui::init_text()
{
    info_string = std::make_shared<text_renderer::Renderable_text>();
    info_string->set_position(glm::vec3(10.0,10.0,0.0));
    info_string->set_color(glm::vec4(1.0,1.0,1.0,1.0));
    info_string->set_scale(0.7f);
    info_string->set_text("0 fps");
    info_string->rendering_state.enable();
    info_string->view_configuration.camera_space();
    renderer->add_renderable( info_string );
}

opengl_ui::opengl_ui(int win_width,
                     int win_heigth) :
    win_h{ win_heigth },
    win_w{ win_width }
{
    LOG1("Creating window, ",win_width,"/",win_heigth);
    //Setup GLFW
    if(glfwInit() != GLFW_TRUE)
    {
        ERR("Unable to initialize GLFW");
        throw std::runtime_error("GLFW init failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Create the window
    window_ctx = glfwCreateWindow(win_width,
                                  win_heigth,
                                  "opengl play",
                                  nullptr,
                                  nullptr);
    if(window_ctx == nullptr){
        ERR("Unable to create the window!");
        throw std::runtime_error("Window creation failed");
    }

    glfwMakeContextCurrent(window_ctx);

    //Init GLEW
    glewExperimental = GL_TRUE;
    GLenum glew_init_status = glewInit();
    if(glew_init_status != GLEW_OK){
        ERR("Unable to initialize GLEW: ",
            glewGetErrorString(glew_init_status));
        throw std::runtime_error("GLEW Init failed!");
    }

    camera = factory< scene::Camera >::create(
                glm::vec3(4.0,4.0,8),
                glm::vec3(0.0,0.0,0.0)
                );
    camera->eagle_mode();

    for(auto& elem:key_status)
        elem = key_status_t::not_pressed;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    //Enable the mouse cursor
    cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    glfwSetCursor(window_ctx, cursor);
    glfwSetInputMode(window_ctx,GLFW_CURSOR,GLFW_CURSOR_NORMAL);

    projection = glm::perspective(glm::radians(45.0f),
                                  (GLfloat)win_w / (GLfloat)win_h,
                                  1.0f, 100.0f);
    glm::mat4 def_ortho = glm::ortho(0.0,
                            (double)win_w,
                            0.0,
                            (double)win_h,
                            0.0,100.0);

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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
}

void opengl_ui::prepare_for_main_loop()
{
    LOG3("Preparing the remaining environment.");
    //Save the instance pointer
    ui_instance = this;
    //Init the callbacks
    setup_callbacks();

    get_current_ctx_viewport();
}

void opengl_ui::setup_scene()
{
    //Register the camera as movable object
    movable::key_mapping_vec camera_keys = {
        { GLFW_KEY_W, { movable::mov_direction::top, { 0.5 } } },
        { GLFW_KEY_S, { movable::mov_direction::down, { 0.5 } } },
        { GLFW_KEY_A, { movable::mov_direction::left, { 0.3 } } },
        { GLFW_KEY_D, { movable::mov_direction::right, { 0.3 } } },
        { GLFW_KEY_Q, { movable::mov_direction::rotate_left, { 2.0 } } },
        { GLFW_KEY_E, { movable::mov_direction::rotate_right, { 2.0 } } },
    };


    movable::mouse_mapping_vec camera_mouse = {
        { movable::mouse_movement_types::wheel_up, { movable::mov_direction::forward, { 0.05 } } },
        { movable::mouse_movement_types::wheel_down, { movable::mov_direction::backward, { 0.05 } } },
    };

    movement_processor.register_movable_object(camera,camera_keys);
    movement_processor.register_movable_object(camera,camera_mouse);

    game_terrain = factory< game_terrains::Terrains >::create(
                renderer::Core_renderer_proxy( renderer ) );


    game_terrain->load_terrain("../models/Grass/grass.obj",
                               glm::vec4(1.0),
                               1);

    game_terrain->load_terrain("../models/Grass/grass2.obj",
                               glm::vec4(1.0),
                               2);

    long mountain_id = game_terrain->load_terrain("../models/Mountain/mountain.obj",
                               glm::vec4(1.0),
                               3);
    game_terrain->load_highres_terrain("../models/Mountain/mountain_highres.obj",
                               mountain_id);

    long forest_id = game_terrain->load_terrain("../models/Forest/Forest.obj",
                               glm::vec4(1.0),
                               4);

    game_terrain->load_highres_terrain("../models/Forest/Forest_complex.obj",
                               forest_id);


    //Generate random terrain map
    const int map_size_x{ 10 };
    const int map_size_y{ 10 };

    std::random_device rd;
    std::mt19937_64 eng( rd() );
    std::uniform_int_distribution<long> dist(1,4);

    game_terrains::terrain_map_t terrain_map;
    terrain_map.resize( map_size_y );
    for( int y{ 0 } ; y < map_size_y ; ++y ) {
        terrain_map[ y ].resize( map_size_x );
        for( int x{ 0 } ; x < map_size_x ; ++x ) {
            terrain_map[ y ][ x ] = dist( eng );
        }
    }


    game_terrain->load_terrain_map( terrain_map,
                                    2,
                                    glm::vec2(map_size_x / 2,
                                              map_size_y / 2) );

    units = factory< game_units::Units >::create(
                renderer::Core_renderer_proxy( renderer ) );

    auto list_of_units = units->buildable_units();
    uint64_t unit_id = list_of_units.front().id;
    auto new_unit = units->create_unit( unit_id );
    auto lot = game_terrain->find_lot( glm::vec2(0.0) );
    if( lot != nullptr ) {
        units->place_unit( new_unit, lot );
    } else {
        PANIC("LOT NOT FOUND!");
    }

    light_1 = lighting::Light_factory<lighting::directional_light>::create(
                glm::vec3(30,30,30),
                glm::vec4(0.9,0.8,0.7,1.0),
                30);

    light_2 = lighting::Light_factory<lighting::directional_light>::create(
                glm::vec3(-10,100,-10),
                glm::vec4(0.8,0.7,0.7,1.0),
                12);

    renderer->scene_lights()->add_light( light_1 );
    renderer->scene_lights()->add_light( light_2 );
}


void opengl_ui::enter_main_loop()
{
    setup_scene();

    auto ref_time = std::chrono::system_clock::now();
    int  current_fps = 0;

    LOG2("Entering main loop!");
    std::string current_fps_string = "0 fps";
    long num_of_rendering_cycles{ 0 };
    while(!glfwWindowShouldClose(window_ctx))
    {
        ++current_fps;
        glfwPollEvents();
        evaluate_key_status();
        movement_processor.process_movements();

        auto current_time = std::chrono::system_clock::now();
        if(std::chrono::duration_cast<
           std::chrono::milliseconds>(
               current_time - ref_time).count() > 1000){
            ref_time = current_time;
            current_fps_string = (std::to_string(current_fps) + " fps");
            current_fps = 0;
        }

        renderer->clear();
        num_of_rendering_cycles = renderer->render();

        auto yaw = camera->get_yaw(),
                pitch = camera->get_pitch(),
                roll = camera->get_roll();
        glm::vec3 pos = camera->get_position();

        std::stringstream ss;
        ss <<current_fps_string<<" - "<<std::setprecision(2)<<std::fixed<< "yaw:"<<yaw<<", pitch:"<<pitch<<", roll:"<<roll
          <<". x:"<<pos.x<<",y:"<<pos.y<<",z:"<<pos.z<<", rendr cycles:"<<num_of_rendering_cycles;

        info_string->set_text(ss.str());

        glfwSwapBuffers(window_ctx);
    }
}

GLFWwindow *opengl_ui::get_win_ctx()
{
    return window_ctx;
}

void opengl_ui::update_viewport(int new_win_h,
                                int new_win_w)
{
    win_h = new_win_h;
    win_w = new_win_w;

    glViewport(0,0,
               win_w,
               win_h);

    viewport = glm::vec4(0.0,0.0,
                         win_w,win_h);
}

opengl_ui::~opengl_ui()
{
    if( nullptr != window_ctx ){
        glfwTerminate();
    }
    if( nullptr != cursor) {
        glfwDestroyCursor(cursor);
    }
}

/*
 * Calculate and return the vector the a ray
 * casted from x,y.
 */
types::ray_t opengl_ui::ray_cast(GLdouble x, GLdouble y)
{
    glm::vec3 ms( x, win_h - y, 0 );
    auto cam_view = camera->get_view();

    glm::vec3 src = glm::unProject(
                ms,
                cam_view,
                projection,
                viewport);

    ms.z = 1;

    glm::vec3 dst = glm::unProject(
                ms,
                cam_view,
                projection,
                viewport);

    return { src, glm::normalize( dst - src ) };
}

glm::vec2 opengl_ui::ray_z_hit_point(const types::ray_t &ray,
                                     const GLfloat z_value)
{
    glm::vec3 target = ray.first;
    GLfloat l = 0,
            r = 1024; //Hopefully is big enough!
    while( l < r ) {
        GLfloat mid = ( r + l ) / 2;
        target = ray.first + ray.second * mid;
        if( glm::abs( target.z - z_value ) <= 0.00001 ) {
            //Looks like 0 :)
            break;
        }
        if( target.z > 0 ) {
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
    log_inst.set_thread_name("MAIN");
    log_inst.set_logging_level( logging::severity_type::debug2 );

    opengl_play::opengl_ui entry(1920,1280);
    entry.prepare_for_main_loop();
    entry.enter_main_loop();
}
