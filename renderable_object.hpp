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
#include <factory.hpp>

namespace renderer
{

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

/*
 * Small utility object used
 * to track the Rendering status
 * of the renderable objects.
 *
 * If the Rendering_state is set to
 * enable then the Renderable will
 * be processed by the Core_renderer
 */
class Rendering_state
{
public:
    enum class states
    {
        rendering_enabled,
        rendering_disabled,
        not_visible, //The rendering is enabled but the Renderable is not visible
    };
public:
    Rendering_state() :
        current_state{ states::rendering_disabled }
    {}
    Rendering_state( const states cur_state ) :
        current_state{ cur_state }
    {}
    void set_enable() {
        current_state = states::rendering_enabled;
    }
    void set_disable() {
        current_state = states::rendering_disabled;
    }
    void set_not_visible() {
        current_state = states::not_visible;
    }
    states current() const {
        return current_state;
    }
private:
    states current_state;
};

/*
 * Small utility object which
 * handle the supported view methods
 */
class View_config
{
public:
    /*
     * Specify how to calculate the position
     * of the object, if it should be in world position
     * or always in front of the camera
     */
    enum class supported_configs
    {
        world_space_coord,
        camera_space_coord //Only X,Y are relevant. Draw in front of the camera
    };
    void configure( supported_configs new_config ) {
        current_setting = new_config;
    }
    supported_configs current() {
        return current_setting;
    }
    bool is_world_space() {
        return current_setting == supported_configs::world_space_coord;
    }
    bool is_camera_space() {
        return current_setting == supported_configs::camera_space_coord;
    }
    void world_space() {
        current_setting = supported_configs::world_space_coord;
    }
    void camera_space() {
        current_setting = supported_configs::camera_space_coord;
    }
private:
    supported_configs current_setting;
};

/*
 * In one place all the rendering data
 */
struct Renderable_data
{
    shaders::Shader::raw_poiner shader;
    /*
     * Transformation matrices used
     * for rendering purpose
     */
    glm::mat4 model_matrix;
    /*
     * Position of the renderable
     */
    glm::vec3 position;
    /*
     * Default color applicable to the model
     */
    types::color default_color;

    Renderable_data() :
        model_matrix{ glm::mat4() }
    {}
    /*
     * Utility functions
     */
    glm::vec3 update_pos_from_model_matrix() {
        position = glm::vec3(
                    model_matrix[3].x,
                    model_matrix[3].y,
                    model_matrix[3].z
                    );
        return position;
    }
};

class Renderable
{
public:
    using pointer = std::shared_ptr< Renderable >;
    using raw_pointer = Renderable*;
    Renderable_data rendering_data;
    Rendering_state rendering_state;
    View_config     view_configuration;

    Renderable();

    void set_shader( shaders::Shader::raw_poiner shader );
    virtual void prepare_for_render( ) {}
    virtual void render( ) {}
    virtual void clean_after_render( ) {}

    virtual std::string nice_name();
    /*
     * Unique ID
     */
    const id_factory< Renderable > id;

    virtual ~Renderable() {}
};

/*
 * Implementes the rendering functionality
 */
struct Rendr
{
    using pointer = std::shared_ptr< Rendr >;
    using raw_pointer = Rendr*;
    id_factory< Rendr > id;
    Renderable::raw_pointer object;

    Rendr() = default;
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
 * Information about a selected
 * model
 */
struct Selected_model_info
{
    using pointer = std::shared_ptr< Selected_model_info >;
    using container = std::vector< pointer >;
    /*
     * Selected models have a slightly different
     * color, the original color is stored here in order
     * to restore it when the model is unselected
     */
    const types::color  original_color;
    Renderable::pointer object;

    Selected_model_info( const types::color color,
                         Renderable::pointer pointer ) :
        original_color{ color },
        object{ pointer }
    {}
};

/*
 * Manage all the selected models
 */
class Selected_models
{
public:
    void add( Renderable::pointer object );
    bool remove( Renderable::pointer object );
    std::size_t removel_all();
    std::size_t count() const;
    const Selected_model_info::container& get_selected();
    bool is_selected( const Renderable::pointer& obj );
private:
    Selected_model_info::container::iterator find( const Renderable::pointer& obj );
    Selected_model_info::container selected;
};

/*
 * Currently pointed model
 */
struct Pointed_model_data
{
    //Screen coordinates
    GLuint x;
    GLuint y;
    /*
     * Set to true if the pointed
     * model need to be updated
     */
    bool update_required;
    //Actual pointed model;
    Renderable::pointer pointed;
};

/*
 * Entry for a pick request. Those
 * structs are generated for each 'pick'
 * coming from the UI.
 */
enum class pick_type
{
    simple,
    toggle
};
struct Pick_request_data
{
    GLuint x;
    GLuint y;
    pick_type type;
};

/*
 * Implements the 'mouse picking' functionality,
 * allows for highlighting of a specific model.
 */
class Model_picking
{
public:
    using pointer = std::shared_ptr< Model_picking >;

    Model_picking(shaders::Shader::pointer shader,
                   buffers::Framebuffers::pointer framebuffers );
    /*
     * Add an additional model which might
     * be 'picked'. return the color assigned
     * to the model.
     */
    types::color add_model( Renderable::pointer object );
    /*
     * Return the model at position x,y
     */
    Renderable::pointer model_at_position( const GLuint x, const GLuint y );
    /*
     * Set and Get the currently pointed model
     */
    void set_pointed_model( const GLuint x, const GLuint y );
    Renderable::pointer get_pointed_model() const;
    /*
     * Given the position x,y returns the selected model
     * (if any), the model will have the default color
     * changed
     */
    std::size_t pick( const GLuint x, const GLuint y );
    /*
     * Work as the usual pick, but if one attempt to select
     * twice the same renderable then the second selection
     * works as unselection
     */
    std::size_t pick_toggle( const GLuint x, const GLuint y );
    /*
     * If any model is currently selected, unselect it
     */
    void unpick();
    /*
     * Return all the currently selected models
     */
    std::vector< Renderable::pointer > get_selected();
    std::vector< types::id_type > get_selected_ids();
    /*
     * Update the picking information for the provided model
     */
    void update(const Renderable::raw_pointer object ) const;
    /*
     * Two functions which ask Model_picking to be ready
     * for rendering next, or to cleanup after the rendering
     * is completed
     */
    void prepare_to_update();
    void complete_update();
private:
    shaders::Shader::pointer           game_shader;
    GLuint                             shader_color_loc;
    buffers::Framebuffers::pointer     framebuffers;
    buffers::Framebuffers::buffer_id_t picking_buffer_id;
    /*
     * List of points for which was submitted
     * a 'pick' operation
     */
    void process_pick_requests();
    std::vector< Pick_request_data > pick_requests;
    /*
     * Container of currently selected models
     */
    Selected_models     selected;
    Pointed_model_data  pointed_model;
    Renderable::pointer pointed;
    Color_creator       color_operations;
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
 * Container for all the configuration settings
 * that do not need to spam the name space
 * of Core_renderer
 */
struct Core_renderer_config
{
    bool             is_def_view_matrix_loaded;
    glm::mat4        view_matrix;
    types::win_size  viewport_size;
    perspective_type cur_perspective;
    glm::mat4        projection;
    glm::mat4        ortho;
    GLint            color_loc;
    GLint            view_loc;
    GLint            projection_loc;
    GLint            model_loc;

    Core_renderer_config( types::win_size win_size ) :
        is_def_view_matrix_loaded{ false },
        viewport_size{ win_size }
    {}
};

/*
 * This buffer will contain all the renderable
 * that we can render. When a new renderable is added
 * to the core renderer it ends up in this buffer.
 *
 * This is a very heavily accessed structure so using
 * vectors does not come into play (the [] operator overhead is
 * not acceptable)
 */
class Rendr_data_buffer
{
public:
    Rendr_data_buffer();

    std::unique_ptr< Rendr::raw_pointer[] > rendr_content_buffer;
    Rendr::raw_pointer* rendr_content;

    /*
     * Depending on the type of camera configuration
     * the elements are put in front or at the back
     * of the current set of renderables in the buffer.
     */
    void add_new_rendr( Rendr::pointer& rendr );
    /*
     * This is public to avoid setter/getter, do not mess anything!! :)
     */
    /*
     * Used for rendering the rendr objects based
     * on their priority (head first, tail last)
     */
    std::size_t buffer_head;
    std::size_t buffer_tail;
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
            const scene::Camera::pointer cam );
    types::id_type add_renderable( Renderable::pointer object );
    long render();
    lighting::lighting_pointer scene_lights();
    Model_picking::pointer     picking();
    /*
     * Clean the rendering buffers
     */
    void clear();
private:
    /*
     * This function prepare the current model for
     * rendering, setup model matrix, view matrix
     * and performs any other step needed to render
     * the Renderable.
     *
     * The function returns false if the Rendering
     * of the Renderable should be interrupted
     */
    bool prepare_for_rendering( Rendr::raw_pointer cur );
    /*
     * Setup the proper color for the Renderable and
     * load it to the shader
     */
    void prepare_rendr_color(Rendr::raw_pointer cur) const;
    Core_renderer_config     config;
    shaders::Shader::pointer shader;
    scene::Camera::pointer   camera;
    scene::Frustum::pointer  frustum;
    scene::Frustum::raw_pointer frustum_raw_ptr;//Save some performance.
    lighting::lighting_pointer     game_lights;
    buffers::Framebuffers::pointer framebuffers;
    Rendr_data_buffer rendr_data;
    /*
     * For fast retrieval of renderable objects
     * by their ID
     */
    std::unordered_map< types::id_type, Rendr::pointer > renderables;
    /*
     * Load the proper perspective matrix
     * to the shader
     */
    void switch_proper_perspective(const Renderable::raw_pointer obj );

    Model_picking::pointer model_picking;
    /*
     * Rendering index buffer, used to store the indexes
     * of the visible renderable in rendering_content
     */
    types::id_type rendering_content_idx_buffer[ 1000 ];
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
    /*
     * The pointed mode is everything which is
     * currently 'under' the mouse
     */
    Renderable::pointer pointed_model() const;

private:
    Core_renderer::pointer core_renderer;
};

}

#endif
