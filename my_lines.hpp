#include <headers.hpp>
#include "logger/logger.hpp"
#include <renderable_object.hpp>
#include <movable_object.hpp>
#include <shaders.hpp>

namespace opengl_play
{


class my_simple_lines;
using my_lines_ptr = std::shared_ptr<my_simple_lines>;

struct single_line
{
    GLfloat from[3],
            f_color[3],
            to[3],
            t_color[3];
};

class my_simple_lines :
        public renderer::Renderable
{
public:
    my_simple_lines();
    ~my_simple_lines();
    int add_line(glm::vec3 from, glm::vec3 to,glm::vec3 color);
    void set_transformations(glm::mat4 view, glm::mat4 projection);
    void modify_view(glm::mat4 new_view);
    void prepare_for_render();
    void render();
    void clean_after_render();
    std::string nice_name();
    const single_line*const get_line_info( int id ) const;
    /*
     * Allows to modify the line 'from' and
     * 'to' values.
     */
    bool modify_line_endpoints( int id,
                                const glm::vec3& origin,
                                const glm::vec3& end);
    int get_invalid_id();
private:
    void update_buffers();
private:
    int next_line_id;
    shaders::my_small_shaders shaders;
    std::vector<single_line> lines;
    GLuint VAO,VBO;
    glm::mat4 view,
    projection;
    /*
     * Map a given line ID with
     * the relative index inside 'lines'
     */
    std::unordered_map<int,int> line_data_idx;
};

}
