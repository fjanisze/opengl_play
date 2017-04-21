#ifndef RENDERABLE_OBJECT_HPP
#define RENDERABLE_OBJECT_HPP

#include <headers.hpp>
#include <vector>
#include <list>
#include <shaders.hpp>
#include <unordered_map>
#include <my_camera.hpp>
#include <lights.hpp>

namespace renderable
{

class renderable_object;

using renderable_pointer = std::shared_ptr< renderable_object >;

/*
 * Renderable objects are classified by
 * priority. The priority influence the
 * precendence of drawing
 */
struct rendr_class
{
    rendr_class() = default;
    rendr_class( std::size_t prio ) :
        priority{ prio } {}
    /*
     * The lower the better
     */
    std::size_t priority{ 5 };
    std::vector<renderable_object*> renderables;
};

enum class renderable_state
{
    rendering_enabled,
    rendering_disabled
};

/*
 * Specify how to calculate the position
 * of the object, if it should be in world position
 * or always in front of the camera
 */
enum class view_method
{
    world_space_coord,
    camera_space_coord //Only X,Y are relevant. Draw in front of the camera
};

class renderable_object
{
public:
    renderable_object();
    void set_rendering_state( const renderable_state new_state );
    renderable_state get_rendering_state();

    virtual void prepare_for_render( shaders::shader_ptr& shader ) {}
    virtual void render( shaders::shader_ptr& shader ) {}
    virtual void clean_after_render( shaders::shader_ptr& shader ) {}

    /*
     * Handy function which permit to force the
     * renderer to not apply the view transformation
     * matrix during rendering.
     */
    void set_view_method( const view_method new_method );
    view_method get_view_method() const {
        return type_of_view;
    }

    virtual std::string renderable_nice_name();

    virtual ~renderable_object() {}

    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;
    glm::mat4 model_matrix;
    glm::vec4 default_color;
    view_method type_of_view;
private:
    renderable_state state;
};

/*
 * Implementes the rendering functionality
 */

using renderable_id = long;

struct rendr;
using rendr_ptr = std::shared_ptr<rendr>;
struct rendr
{
    renderable_id id;
    renderable_pointer object;
    /*
     * For rendering purpose we have those
     * rendr objects inside a list, the head
     * elements should be rendered first, the tail
     * last..
     */
    rendr_ptr next;

    rendr():
        id{ -1 }
    {}
};

/*
 * Two perspective models are supported:
 *  projection: For common drawing
 *  ortho: Mostly for rendering stuff in front of the camera.
 */
enum class perspective_type
{
    projection,
    ortho
};

class core_renderer
{
public:
    core_renderer() = default;
    core_renderer(const glm::mat4& proj , const glm::mat4 &def_ortho,
                  const opengl_play::camera_ptr cam );
    renderable_id add_renderable( renderable_pointer object );
    long render();
    lighting::lighting_pointer lights();
private:
    shaders::shader_ptr shader;
    opengl_play::camera_ptr camera;
    lighting::lighting_pointer game_lights;
    perspective_type cur_perspective;
    glm::mat4 projection;
    glm::mat4 ortho;
    renderable_id next_rendr_id;
    /*
     * Used for rendering the rendr objects based
     * on their priority (head first, tail last)
     */
    rendr_ptr rendering_head;
    rendr_ptr rendering_tail;
    /*
     * For fast retrieval of renderable objects
     * by their ID
     */
    std::unordered_map< renderable_id, rendr_ptr > renderables;
    GLint load_location( const std::string& loc_name );
    /*
     * Load the proper perspective matrix
     * to the shader
     */
    void switch_proper_perspective(const renderable_pointer &obj );
private:
    GLint color_loc;
    GLint view_loc;
    GLint projection_loc;
    GLint model_loc;
};

using renderer_pointer = std::shared_ptr< core_renderer >;

}

#endif
