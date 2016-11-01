#include "opengl_play.hpp"

namespace opengl_play
{

namespace
{
    //Saved context for the GLFW callbacks
    opengl_ui* ui_instance;
    //Global for the mouse callback
    int left_button_state = GLFW_RELEASE;
    //This coordinate is needed to calculate the delta movement
    glm::dvec2 last_know_mouse_position = {0,0};
}

GLfloat mycos(double val){
    return (GLfloat)cos(val) * 0.5;
}


GLfloat mysin(double val){
    return (GLfloat)sin(val) * 0.5;
}

GLfloat torad(double val){
    return val * M_PI/180;
}

void opengl_ui::mouse_click_callback(GLFWwindow *ctx,
                                     int button,
                                     int action,
                                     int)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
        {
            left_button_state = action;
            glfwGetCursorPos(ui_instance->get_win_ctx(),
                             &last_know_mouse_position.x,
                             &last_know_mouse_position.y);
        }
        else
            left_button_state = GLFW_RELEASE;
    }
}

void opengl_ui::cursor_pos_callback(GLFWwindow *ctx,
                                    double x,
                                    double y)
{
    if(left_button_state == GLFW_PRESS)
    {
        double delta_x = x - last_know_mouse_position.x,
               delta_y = y - last_know_mouse_position.y; //y not used yet

        rotation_direction dir = delta_x > 0 ? rotation_direction::right :
                                               rotation_direction::left;

        ui_instance->rotate_triangle(dir, delta_x);

        last_know_mouse_position.x = x;
        last_know_mouse_position.y = y;
    }
}

void opengl_ui::window_resize_callback(GLFWwindow *ctx,
                                       int width,
                                       int height)
{
    ui_instance->update_viewport(height,width);
    ui_instance->image_update_needed();
}

void opengl_ui::rotate_triangle(rotation_direction dir,
                                int amount)
{
    for(int i{0};i<3;i++)
    {
        my_triangle[i].x = ( my_triangle[i].x + amount ) % 360;
        my_triangle[i].y = ( my_triangle[i].y + amount ) % 360;
    }

    if(points_count>0){
        int i = points_count * 3 - 1;
        for(;( i - 3 ) >= 3 * 3 ;--i)
        {
            vertices[ i ] = vertices[ i - 3 ];
        }
    }

    vertices[ 3 * 3     ] = vertices[0];
    vertices[ 3 * 3 + 1 ] = vertices[1];
    vertices[ 3 * 3 + 2 ] = vertices[2];

    points_count = std::min(points_count + 1,
                            AMOUNT_OF_POINTS);
    //After every rotation an update is needed
    render_update_needed = true;
}

void opengl_ui::setup_callbacks()
{

    glfwSetMouseButtonCallback(window_ctx,
                               mouse_click_callback);
    glfwSetCursorPosCallback(window_ctx,
                             cursor_pos_callback);
    glfwSetWindowSizeCallback(window_ctx,
                              window_resize_callback);
}

void opengl_ui::get_current_ctx_viewport()
{
    glfwGetFramebufferSize(window_ctx,
                           &win_w,
                           &win_h);
    glViewport(0,0,
               win_w,
               win_h);

    update_win_size_infotext();
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

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    triangle_color.x = 1.0f;
    triangle_color.y = 1.0f;
    triangle_color.z = 1.0f;
}

void opengl_ui::prepare_for_main_loop()
{
    LOG3("opengl_ui, preparing the remaining environment.");
    check_for_errors();
    init_my_triangle();

    //Add text info text
    window_size_text = std::make_shared<text_renderer::renderable_text>();
    window_size_text->set_color(glm::vec3(.5f,1,0));
    window_size_text->set_position(glm::fvec2(50,50));
    window_size_text->set_scale(.5f);
    window_size_text->set_text("");
    window_size_text->set_window_size(win_h,
                           win_w);

    //Add text info text
    position_info_text = std::make_shared<text_renderer::renderable_text>();
    position_info_text->set_color(glm::vec3(1,1,1));
    position_info_text->set_position(glm::fvec2(50,100));
    position_info_text->set_scale(.35f);
    position_info_text->set_text("");
    position_info_text->set_window_size(win_h,
                           win_w);

    //Save the instance pointer
    ui_instance = this;
    //Set to true if there's something to update
    //is true now since we need an initial update
    render_update_needed = true;
    //Init the callbacks
    setup_callbacks();

    get_current_ctx_viewport();
}

void opengl_ui::init_my_triangle()
{
    my_triangle[0] = glm::ivec2(90,90),
    my_triangle[1] = glm::ivec2(225,225),
    my_triangle[2] = glm::ivec2(315,315);

    points_count = 0;
}


void opengl_ui::update_vertices()
{
    //I need to coordinates to be in NDC format
    //(Normalized Device Coordinates)
    std::size_t tr_idx = 0;
    for(std::size_t i = 0;i<9;i+=3)
    {
        vertices[i    ] = mycos(torad(my_triangle[tr_idx].x));
        vertices[i + 1] = mysin(torad(my_triangle[tr_idx].y));
        vertices[i + 2] = 0;
        ++tr_idx;
    }
    //Update the text information
    GLfloat x_text_pos = vertices[0]*win_w/2 + win_h/2,
            y_text_pos = vertices[1]*win_h/2 + win_h/2;
    position_info_text->set_position(glm::fvec2(x_text_pos,y_text_pos));
    std::stringstream new_text;
    new_text << "X:" << x_text_pos<<",Y:"<<y_text_pos;
    position_info_text->set_text(new_text.str());
}

void opengl_ui::update_triangle_color()
{
    static const float color_delta = 0.01f;
    static int idx = 0;
    float* colors[3] = { &triangle_color.x,
                         &triangle_color.y,
                         &triangle_color.z };
    if(*(colors[idx]) - color_delta < 0){
        *(colors[idx]) = 0;
        if(idx == 2){
            triangle_color = glm::vec3(1.0,1.0,1.0);
            idx = 0;
        }
        else ++idx;
    }else{
        *(colors[idx]) -= color_delta;
    }
    //Upload the new uniform information
    GLint vertex_colors = glGetUniformLocation(shaders.get_program(),
                                               "selected_color");
    if(vertex_colors < 0){
        ERR("Unable to load the uniform \"selected_color\"");
    }
    else{
        glUseProgram(shaders.get_program());
        glUniform4f(vertex_colors,triangle_color.x,
                    triangle_color.y,
                    triangle_color.z,
                    1.0f);
    }
}

void opengl_ui::update_win_size_infotext()
{
  //  window_size_text->set_window_size(win_h,win_w);
  //  position_info_text->set_window_size(win_h,win_w);
    std::stringstream text;
    text << "Window size: "<<win_h<<"/"<<win_w;
    window_size_text->set_text(text.str());
}


int opengl_ui::check_for_errors()
{
    int error_count = 0;
    GLenum error = GL_NO_ERROR;

    while( (error = glGetError()) != GL_NO_ERROR){
        ERR("OpenGL ERROR in opengl_ui: ",
            error);
        ++error_count;
    }
    return error_count;
}

void opengl_ui::enter_main_loop()
{
    //We need to load the shaders
    shaders.load_fragment_shader(simple_fragment_shader);
    shaders.load_vertex_shader(simple_vertex_shader);

    shader2.load_fragment_shader(simple_fragment_shader2);
    shader2.load_vertex_shader(simple_vertex_shader);
    if(!shaders.create_shader_program() || !shader2.create_shader_program()){
        return;
    }

    //First update
    update_vertices();
    update_triangle_color();

    //We need to create a vertice buffer object
    GLuint vertices_vbo;
    GLuint vertices_vao;
    glGenBuffers(1, //Number of vertice bufers
                 &vertices_vbo);
    glGenVertexArrays(1, //number of vertex array objects
                      &vertices_vao);
    //bind the vao
    glBindVertexArray(vertices_vao);
    //Now we want to load those vertices
    glBindBuffer(GL_ARRAY_BUFFER,
                 vertices_vbo);
    //Copy the data to the GPU memory
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertices),
                 vertices,
                 GL_DYNAMIC_DRAW);

    /*
     * Now we need to link the vertex attributes
     * Those are needed for the vertex shader input.
     * In the sample VS we have a vertex attribute 'position'
     * at location 0, so we will target the attribute 0
     */
    glVertexAttribPointer(0, //Attribute 0 (position)
                          3, //size of the vertex attribute, 3 vertex
                          GL_FLOAT, //type of the attribute
                          GL_FALSE, //no need to normalize
                          3 * sizeof(GLfloat), //stride size (we have 3 vertex, each 3*float)
                          nullptr);//Offset where the data begin, 0 in our case
    glEnableVertexAttribArray(0); //the location is 0, look at the VS

    //unbind the vao, we're ready to go
    glBindVertexArray(0);

    check_for_errors();
    //now we have our object configured and ready to be rendered

    LOG2("Entering main loop!");
    while(!glfwWindowShouldClose(window_ctx))
    {
        glfwPollEvents();

        //Do not draw anythin if is not needed
        if(false == render_update_needed)
        {
            continue;
        }

        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        //Enable the shaders
        shaders.use_shaders();
        //All we need to do is bind the vao which store
        //all our attribute for the object we want to draw
        glBindVertexArray(vertices_vao);

        glDrawArrays(GL_TRIANGLES,//Type of primitive to draw
                     0,//Start index in the vertex array, is 0 in our case
                     3); //Amount of vertices, we have 3 vertices

        shader2.use_shaders();
        glDrawArrays(GL_POINTS,
                     3,
                     AMOUNT_OF_POINTS);

        //When we're done unbind the vao
        glBindVertexArray(0);

        //Draw the info text
        window_size_text->render_text();
        position_info_text->render_text();

        glfwSwapBuffers(window_ctx);


        //Update the triangle data
        update_vertices();
        update_triangle_color();

        glBindBuffer(GL_ARRAY_BUFFER,
                     vertices_vbo);

        //Update the vao buffer
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        sizeof(vertices),
                        vertices);

        //Till the next update
        render_update_needed = false;
    }
}

GLFWwindow *opengl_ui::get_win_ctx()
{
    return window_ctx;
}

void opengl_ui::image_update_needed()
{
    render_update_needed = true;
}

void opengl_ui::update_viewport(int new_win_h,
                                int new_win_w)
{
    win_h = new_win_h;
    win_w = new_win_w;

    glViewport(0,0,
               win_w,
               win_h);

    update_win_size_infotext();
}

opengl_ui::~opengl_ui()
{
    if(window_ctx!=nullptr){
        glfwTerminate();
    }
}

}

int main()
{
    opengl_play::opengl_ui entry(800,600);
    log_inst.set_thread_name("MAIN");

    entry.prepare_for_main_loop();
    entry.enter_main_loop();
}
