#include <headers.hpp>
#include <shaders.hpp>
#include <set>

namespace opengl_play
{

int check_for_errors();

typedef unsigned char byte_t;

struct vertex_info
{
	GLfloat x,y,z;
	GLfloat t_x,t_y;//Texture coordinates
};

struct texture_info
{
	GLuint texture;
	GLint  width, height;
	operator GLuint(){
		return texture;
	}
};

enum class rotation_axis
{
	x,
	y,
	z
};

struct object_data
{
	glm::vec3 position;
	glm::mat4 model,
			view,
			projection;
};

class little_object
{
	GLuint VAO,VBO;
	GLfloat current_mix_ratio;
	std::map<std::string,texture_info> textures;
	GLint  tex_width,tex_height;
	vertex_info vertices[36];
	GLfloat   vertex_data[36*5];
	glm::mat4 camera_view;
	shaders::my_small_shaders obj_shader;

	void init_vertices();
	texture_info load_texture(const std::string& filename,
							  GLint wrapping_method = GL_REPEAT);
	void update_vertex_data();
	std::map<int,object_data> objects;
	int next_object_id;
	decltype(objects)::iterator sel_obj_it;
	int selected_object;
	bool any_object_selected();
	void apply_transformations(decltype(objects)::value_type& elem);
	void apply_position(decltype(objects)::value_type& elem);
public:
	little_object();
	~little_object();
	void prepare_for_render();
	void render();
	void clean_after_render();
	void mouse_click(GLint button,GLint action);
	void image_rotation(rotation_axis axis, GLfloat amount);
	void move(mov_direction dir, GLfloat amount);
	glm::vec3 get_object_position();
	void scale(GLfloat amount);
	void set_transformations(glm::mat4 model,glm::mat4 view,glm::mat4 projection);
	int add_object(const glm::vec3& coordinates);
	bool select_object(int id);
	bool release_current_object();
	void modify_view(glm::mat4 new_view);
	std::set<int> get_all_objects();
};

using little_object_ptr = std::shared_ptr<little_object>;

}