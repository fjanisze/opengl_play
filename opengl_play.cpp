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

void opengl_ui::rotate_triangle(rotation_direction dir,
                                int amount)
{
    for(int i{0};i<3;i++)
    {
        my_triangle[i].x = ( my_triangle[i].x + amount ) % 360;
        my_triangle[i].y = ( my_triangle[i].y + amount ) % 360;
    }
}

void opengl_ui::setup_callbacks()
{

    glfwSetMouseButtonCallback(window_ctx,
                               mouse_click_callback);
    glfwSetCursorPosCallback(window_ctx,
                             cursor_pos_callback);
}

void opengl_ui::update_viewport()
{
    glfwGetFramebufferSize(window_ctx,
                           &win_w,
                           &win_h);
    glViewport(0,0,
               win_w,
               win_h);
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
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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

    init_my_triangle();
    //Save the instance pointer
    ui_instance = this;
    //Init the callbacks
    setup_callbacks();
}

void opengl_ui::init_my_triangle()
{
    my_triangle[0] = glm::ivec2(90,90),
    my_triangle[1] = glm::ivec2(225,225),
    my_triangle[2] = glm::ivec2(315,315);
}

void opengl_ui::update_vertices()
{
    std::size_t tr_idx = 0;
    for(std::size_t i = 0;i<9;i+=3)
    {
        vertices[i    ] = mycos(torad(my_triangle[tr_idx].x));
        vertices[i + 1] = mysin(torad(my_triangle[tr_idx].y));
        vertices[i + 2] = 0;
        ++tr_idx;
    }
}

void opengl_ui::enter_main_loop()
{
    //We need to load the shaders
    shaders.load_fragment_shader(simple_fragment_shader);
    shaders.load_vertex_shader(simple_vertex_shader);
    if(!shaders.create_shader_program()){
        return;
    }

    //First update
    update_vertices();

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
    //now we have our object configured and ready to be rendered

    LOG2("Entering main loop!");
    double fps_counter = 0;
    while(!glfwWindowShouldClose(window_ctx))
    {
        glfwPollEvents();
        glClearColor(.5,.5,.5,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        //Enable the shaders
        shaders.use_shaders();
        //All we need to do is bind the vao which store
        //all our attribute for the object we want to draw
        glBindVertexArray(vertices_vao);

        glDrawArrays(GL_TRIANGLES,//Type of primitive to draw
                     0,//Start index in the vertex array, is 0 in our case
                     3); //Amount of vertices, we have 3 vertices

        //When we're done unbind the vao
        glBindVertexArray(0);

        glfwSwapBuffers(window_ctx);

        //Update the coordinates
        update_vertices();

        //Update the vao buffer
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        sizeof(vertices),
                        vertices);
    }
}

GLFWwindow *opengl_ui::get_win_ctx()
{
    return window_ctx;
}

opengl_ui::~opengl_ui()
{
    if(window_ctx!=nullptr){
        glfwTerminate();
    }
}

void my_small_shaders::load_shader_generic(GLuint &shader_target,
                                           const std::string &body,
                                           GLenum shader_type)
{
    const char* body_ptr = body.c_str();
    LOG3("Compiling shader: ",
         shader_type);
    shader_target = glCreateShader(shader_type);
    glShaderSource(shader_target,
                   1,
                   &body_ptr,
                   nullptr);
    glCompileShader(shader_target);
    GLint success;
    glGetShaderiv(shader_target,
                  GL_COMPILE_STATUS,
                  &success);
    if(!success){
        ERR("Compilation failed!");
        glGetShaderInfoLog(shader_target,
                           512,
                           nullptr,
                           log_buffer);
        ERR("Detailed information: ",log_buffer);
    }
}

my_small_shaders::my_small_shaders()
{
    LOG3("Creating the shader compiler");

}

void my_small_shaders::load_vertex_shader(const std::string &body)
{
    load_shader_generic(vertex_shader,
                        body,
                        GL_VERTEX_SHADER);
}

void my_small_shaders::load_fragment_shader(const std::string &body)
{
    load_shader_generic(fragment_shader,
                        body,
                        GL_FRAGMENT_SHADER);
}

bool my_small_shaders::create_shader_program()
{
    LOG3("Creating the shader program");
    shader_program = glCreateProgram();
    //Attach our two shaders
    glAttachShader(shader_program,
                   vertex_shader);
    glAttachShader(shader_program,
                   fragment_shader);
    //Link the shaders
    glLinkProgram(shader_program);
    //Check for errors
    GLint success;
    glGetProgramiv(shader_program,
                  GL_LINK_STATUS,
                  &success);
    if(!success){
        ERR("Linking failed!");
        glGetShaderInfoLog(shader_program,
                           512,
                           nullptr,
                           log_buffer);
        ERR("Detailed information: ",log_buffer);
        return false;
    }
    return true;
}

void my_small_shaders::use_shaders()
{
    glUseProgram(shader_program);
}

}

int main()
{
    opengl_play::opengl_ui entry(800,600);

    //Init GLEW
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        ERR("Unable to initialize GLEW!");
    }
    else
    {
        entry.enter_main_loop();
    }
}
