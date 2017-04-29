#ifndef RENDERABLE_OBJECT_HPP
#define RENDERABLE_OBJECT_HPP

#include <headers.hpp>
#include <vector>
#include <list>
#include <shaders.hpp>
#include <unordered_map>
#include <my_camera.hpp>
#include <lights.hpp>
#include <framebuffers.hpp>
#include <types.hpp>

namespace renderer
{

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

class Renderable
{
public:
    using pointer = std::shared_ptr< Renderable >;

    Renderable();
    void set_rendering_state( const renderable_state new_state );
    renderable_state get_rendering_state() const;

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

    virtual ~Renderable() {}

    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;
    glm::mat4 model_matrix;
    types::color default_color;

    /*
     * Each Renderable shall have its
     * own unique identifiactor
     */
    const uint64_t id;
private:
    renderable_state state;
    view_method type_of_view;
};

/*
 * Implementes the rendering functionality
 */

using renderable_id = long;

struct Rendr;
using rendr_ptr = std::shared_ptr<Rendr>;
struct Rendr
{
    renderable_id id;
    Renderable::pointer object;
    /*
     * For rendering purpose we have those
     * rendr objects inside a list, the head
     * elements should be rendered first, the tail
     * last..
     */
    rendr_ptr next;

    Rendr():
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

/*
 * Model picking works using the color selection
 * method, in which different objects are selected
 * on the base of their uniquely assigned color
 */
class Color_creator
{
public:
    Color_creator( const GLfloat step = 1.0f / 50.0f );
    /*
     * Return a normalized RGBA color where:
     * 0.0 = 0.0 , 1.0 = 255
     */
    types::color get_color();
    /*
     * Convert the RGB range from 0.0-1.0
     * to 0.0-255.0
     */
    types::color denormalize_color( types::color color ) const;
    /*
     * Convert a denormalized color (0.0-255.0)
     * to a normalized color
     */
    types::color normalize_color( types::color color ) const;
    /*
     * Conversion function from color RGBA to codes
     * and vice-versa
     */
    uint64_t get_color_code( const types::color& color );
    types::color get_color_rgba( const uint64_t color_code ) const;
private:
    types::color next_color;
    GLfloat color_step;
    uint64_t num_of_colors;
    uint64_t max_num_of_colors;
};

/*
 * Implements the 'mouse picking' functionality,
 * allows for highlighting of a specific model.
 */
class Model_picking
{
public:
    using pointer = std::shared_ptr< Model_picking >;

    Model_picking( shaders::shader_ptr shader,
                   buffers::Framebuffers::pointer framebuffers );
    /*
     * Add an additional model which might
     * be 'picked'. return the color assigned
     * to the model.
     */
    types::color add_model(Renderable::pointer object );
    /*
     * Given the position x,y returns the selected model
     * (if any), also set the renderable as 'selected' in
     * the Renderable object
     */
    Renderable::pointer pick( const GLuint x, const GLuint y );
    /*
     * If any model is currently selected, unselect it
     */
    void unpick();
    /*
     * Return the ID of the currently selected
     * Renderable, if any
     */
    uint64_t get_selected() const;
    /*
     * Update the picking information for the provided model
     */
    void update( const Renderable::pointer& object );
    /*
     * Two functions which ask Model_picking to be ready
     * for rendering next, or to cleanup after the rendering
     * is completed
     */
    void prepare_to_update();
    void cleanup_after_update();
private:
    shaders::shader_ptr game_shader;
    GLuint shader_color_loc;
    buffers::Framebuffers::pointer framebuffers;
    buffers::Framebuffers::buffer_id_t picking_buffer_id;
    uint64_t cur_selected_renderable;
    Color_creator color_operations;
    /*
     * Map a renderable ID to a color code
     */
    std::unordered_map< uint64_t, uint64_t > rendrid_to_color;
    /*
     * Map a color code to a renderable
     */
    std::unordered_map< uint64_t, Renderable::pointer > color_to_rendr;
};

/*
 * The core rendering object, given a properly
 * configured context it render the models
 * which are selected for rendering
 */
class Core_renderer
{
public:
    using pointer = std::shared_ptr< Core_renderer >;
public:
    Core_renderer() = default;
    Core_renderer(
            const types::win_size& window,
            const glm::mat4& proj,
            const glm::mat4 &def_ortho,
            const opengl_play::camera_ptr cam );
    renderable_id add_renderable( Renderable::pointer object );
    long render();
    lighting::lighting_pointer scene_lights();
    /*
     * This is the wrapper for the mouse picking
     * functionality
     */
    Renderable::pointer select_model( const GLuint x,
                                         const GLuint y );
    /*
     * Return the currently selected model
     */
    uint64_t get_selected_model() const;
    /*
     * Clean the rendering buffers
     */
    void clear();
private:
    shaders::shader_ptr shader;
    opengl_play::camera_ptr camera;
    lighting::lighting_pointer game_lights;
    buffers::Framebuffers::pointer framebuffers;

    types::win_size viewport_size;
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
    /*
     * Load the proper perspective matrix
     * to the shader
     */
    void switch_proper_perspective(const Renderable::pointer &obj );

    GLint color_loc;
    GLint view_loc;
    GLint projection_loc;
    GLint model_loc;

    Model_picking::pointer model_picking;
};

/*
 * Proxy interface for the Core_rendered,
 * should be provided to whoever needs
 * to perform some ALLOWED operations on the renderer,
 * like adding or removing renderable objects
 */
class Core_renderer_proxy
{
public:
    Core_renderer_proxy( Core_renderer::pointer renderer ) :
        core_renderer{ renderer }
    {}
    auto add_renderable( Renderable::pointer&& object ) {
        return core_renderer->add_renderable( std::forward< Renderable::pointer >( object ) );
    }
    uint64_t get_selected_renderable() const {
        return core_renderer->get_selected_model();
    }

private:
    Core_renderer::pointer core_renderer;
};

}

#endif
