#include <headers.hpp>
#include <shaders.hpp>

namespace opengl_play
{

int check_for_errors();

typedef unsigned char byte_t;

struct vertex_info
{
	GLfloat x,y,z;
	GLfloat r,g,b;
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

enum class mov_direction
{
	left,
	right,
	top,
	down
};

class little_object
{
	GLuint VAO,VBO,EBO;
	GLfloat current_mix_ratio;
	std::map<std::string,texture_info> textures;
	GLint  tex_width,tex_height;
	vertex_info vertices[4];
	GLuint    vertex_idxs[6];
	GLfloat   vertex_data[8*4];
	glm::mat4 object_position; //This is just a transformation matrix
	shaders::my_small_shaders obj_shader;

	void init_vertices();
	texture_info load_texture(const std::string& filename,
							  GLint wrapping_method = GL_REPEAT);
	void update_vertex_data();
public:
	little_object();
	~little_object();
	void prepare_for_render();
	void render();
	void clean_after_render();
	void mouse_click(GLint button,GLint action);
	void image_rotation(GLfloat amount);
	void move(mov_direction dir, GLfloat amount);
};

using little_object_ptr = std::shared_ptr<little_object>;

}
