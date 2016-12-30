#include <headers.hpp>
#include "logger/logger.hpp"
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

class my_static_lines
{
	shaders::my_small_shaders shaders;
	std::vector<single_line> lines;
	GLuint VAO,VBO;
	glm::mat4 view,
			projection,
			model;
	void update_buffers();
public:
	my_static_lines();
	~my_static_lines();
	int add_line(glm::vec3 from, glm::vec3 to,glm::vec3 color);
	void modify_view(glm::mat4 new_view);
	void modify_projection(glm::mat4 new_projection);
	void modify_model(glm::mat4 new_model);
	void prepare_for_render();
	void render();
	void clean_after_render();
};

}
