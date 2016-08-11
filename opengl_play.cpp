#include "opengl_play.hpp"

namespace opengl_play
{

void opengl_ui::setup_callbacks()
{

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
}

void opengl_ui::enter_main_loop()
{
    //We need to load the shaders
    shaders.load_fragment_shader(simple_fragment_shader);
    shaders.load_vertex_shader(simple_vertex_shader);
    if(!shaders.create_shader_program()){
        return;
    }
    //Triagle vertex coordinates
    GLfloat vertices[] = {
        -0.5,-0.5,0,
        0.5,-0.5,0,
        0,0.5,0
    };
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
                 GL_STATIC_DRAW);

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
        glBindVertexArray(vertices_vao);

        glfwSwapBuffers(window_ctx);
    }
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
