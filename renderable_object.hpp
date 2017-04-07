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

class renderable_object
{
public:
    virtual void prepare_for_render() {}
    virtual void render( shaders::shader_ptr& shader ) {}
    virtual void clean_after_render() {}
    virtual std::string renderable_nice_name();

    virtual void rotate_object(GLfloat yaw) {}
    virtual ~renderable_object() {}

    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;
    glm::mat4 model_matrix;
    glm::vec3 default_color;
};

/*
 * Implementes the rendering functionality
 */

using renderable_id = long;

struct rendr
{
    renderable_id id;
    renderable_pointer object;
};


class core_renderer
{
public:
    core_renderer() = default;
    core_renderer(const glm::mat4& proj ,
                  const opengl_play::camera_ptr cam );
    renderable_id add_renderable( renderable_pointer object );
    long render();
    lighting::lighting_pointer lights();
private:
    shaders::shader_ptr shader;
    opengl_play::camera_ptr camera;
    lighting::lighting_pointer game_lights;
    glm::mat4 projection;
    renderable_id next_rendr_id;
    std::unordered_map< renderable_id, rendr > renderables;
private:
    GLint color_loc;
    GLint view_loc;
    GLint projection_loc;
    GLint model_loc;
};

using renderer_pointer = std::shared_ptr< core_renderer >;

}

#endif
