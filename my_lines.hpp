#include <headers.hpp>
#include "logger/logger.hpp"
#include <renderable_object.hpp>
#include <movable_object.hpp>
#include <shaders.hpp>

namespace opengl_play
{


class my_static_lines;
using my_lines_ptr = std::shared_ptr<my_static_lines>;

struct single_line
{
	GLfloat from[3],
			f_color[3],
			to[3],
			t_color[3];
};

class my_static_lines :
		public renderable::renderable_object
{
	shaders::my_small_shaders shaders;
	std::vector<single_line> lines;
	GLuint VAO,VBO;
	glm::mat4 view,
			projection;
	void update_buffers();
public:
	my_static_lines();
	~my_static_lines();
	int add_line(glm::vec3 from, glm::vec3 to,glm::vec3 color);
	void set_transformations(glm::mat4 view, glm::mat4 projection);
	void modify_view(glm::mat4 new_view);
	void prepare_for_render();
	void render();
	void clean_after_render();
};

}
