#include <headers.hpp>
#include <shaders.hpp>
#include <lights.hpp>
#include <renderable_object.hpp>
#include <set>

namespace opengl_play
{

int check_for_errors();

typedef unsigned char byte_t;
using movable::mov_direction;

struct vertex_info
{
	GLfloat x,y,z;
	GLfloat t_x,t_y;//Texture coordinates
	GLfloat n_x,n_y,n_z;//Normal vector for diffuse lighting calculation
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
	glm::vec3 color;
	GLfloat scale;
};

class little_object : lights::object_lighting ,
		renderable::renderable_object
{
public:
	little_object();
	~little_object();
	void prepare_for_render();
	void render();
	void clean_after_render();
	/*
	 * Do not render objects which are at a
	 * distance higher than 'limit' from the
	 * selected 'target' (camera?)
	 */
	void limit_render_distance(movable::mov_obj_ptr target,GLfloat radius);
	void object_rotation(rotation_axis axis, GLfloat amount);
	void move(mov_direction dir, GLfloat amount);
	glm::vec3 get_object_position();
	void scale(GLfloat amount);
	void set_transformations(glm::mat4 v,glm::mat4 p);
	void add_object(const glm::vec3& coordinates,const glm::vec3& color,GLfloat scale);
	void modify_view(glm::mat4 new_view);
	std::set<int> get_all_objects();
private:
	GLuint VAO,VBO;
	std::map<std::string,texture_info> textures;
	GLint  tex_width,tex_height;
	vertex_info vertices[36];
	GLfloat   vertex_data[36*8];
	shaders::my_small_shaders obj_shader;

	void init_vertices();
	texture_info load_texture(const std::string& filename,
							  GLint wrapping_method = GL_REPEAT);
	void update_vertex_data();
	std::vector<object_data> objects;
	movable::mov_obj_ptr render_radius_origin;
	GLfloat render_radius;
	int next_object_id;
	decltype(objects)::iterator sel_obj_it;
	void apply_transformations(decltype(objects)::value_type& elem);
	void apply_position(decltype(objects)::value_type& elem);
	void render_with_radius(glm::vec3 origin);
	void render_all();
	glm::mat4 view,
			projection;
};

using little_object_ptr = std::shared_ptr<little_object>;

}
