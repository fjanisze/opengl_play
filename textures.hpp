#include <headers.hpp>
#include <assimp/scene.h>

namespace textures
{


enum class texture_type
{
	diffuse,
	specular,
	unknown
};

struct texture_t
{
	GLuint id;
	GLint  width, height;
	texture_type type;
	std::string path;
	operator GLuint(){
		return id;
	}
};

/*
 * This mapping helps to translate the enum type from ASSIMP
 * to texture_type values
 */
constexpr texture_type map_texture_type(const aiTextureType type) {
	switch( type )
	{
	case aiTextureType_DIFFUSE:
		return texture_type::diffuse;
	case aiTextureType_SPECULAR:
		return texture_type::specular;
	default:
		break;
	}
	return texture_type::unknown;
}

//I'll find a better place for this function at the next refactoring.
texture_t load_texture(const std::string &filename, GLint wrapping_method = GL_REPEAT);

}
