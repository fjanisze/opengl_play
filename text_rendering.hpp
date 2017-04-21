#ifndef TEXT_RENDERING_HPP
#define TEXT_RENDERING_HPP

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
//Generic
#include <string>
#include <unordered_map>
#include <memory>
#include <utility>
//Game
#include "shaders.hpp"
#include "logger/logger.hpp"
#include <renderable_object.hpp>

namespace text_renderer
{

const std::string simple_vertex_shader = {
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec2 tex_coord;\n"
    "out vec2 texture_coords;"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "gl_Position = projection * vec4(position, 1.0);\n"
    "texture_coords = tex_coord;\n"
    "}\0"
};

const std::string simple_fragment_shader = {
    "#version 330 core\n"
    "in vec2 texture_coords;\n"
    "out vec4 color;\n"
    "uniform sampler2D text_rendr_texture;\n"
    "uniform vec3 textColor;\n"
    "void main()\n"
    "{\n"
    "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text_rendr_texture, texture_coords).r);\n"
    "color = vec4(textColor, 1.0) * sampled;\n"
    "}\n\0"
};

// Holds all state information relevant to a character as loaded using FreeType
struct character_data {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::fvec2 Size;    // Size of glyph
    glm::fvec2 Bearing;  // Offset from baseline to left/top of glyph
    GLfloat Advance;    // Horizontal offset to advance to next glyph
};

/*
 * Contains the information required to render a font set
 */
struct font_texture
{
    std::string font_name;
    std::unordered_map<GLchar,character_data> charset;
};

using font_texture_ptr = std::shared_ptr<font_texture>;

/*
 * Instead of passing everywhere the font name
 * the code will identify the various font by
 * a unique ID
 */
using font_type_id = int;

class font_texture_loader
{
    FT_Library freetype_lib;
    font_type_id next_id;
    std::unordered_map<font_type_id,font_texture_ptr> fonts;
    font_type_id default_font_id;
public:
    font_texture_loader();
    ~font_texture_loader();
    std::pair<font_type_id,font_texture_ptr> load_new_textureset(const std::string& font_name);
    font_texture_ptr get_texture(font_type_id id);
    font_type_id get_default_font_id();
};

class Renderable_text;

using rendr_text = std::shared_ptr<Renderable_text>;

class Renderable_text : public renderable::renderable_object
{
public:
    Renderable_text();
    Renderable_text(const std::string& text,
                    const glm::vec3 &position,
                    GLfloat scale,
                    glm::vec4 color); //Use the default font
    void set_window_size(int height,int width);
    void set_text(const std::string& text);
    void set_position(const glm::vec3 position);
    void set_scale(GLfloat scale);
    void set_color(glm::vec4 color);

    void prepare_for_render( shaders::shader_ptr& shader ) override;
    void render( shaders::shader_ptr& shader ) override;
    void clean_after_render( shaders::shader_ptr& shader ) override;
private:
    GLuint VAO,VBO;

    font_texture_loader  font_loader;
    font_texture_ptr     font_texture;

    std::string text_string;
    glm::vec3   text_position;
    GLfloat     text_scale;
    glm::mat4   text_projection;
    GLint       light_calc_uniform;
    int window_height,
    window_width;

    void init();
    void check_for_errors();
};

}

#endif
