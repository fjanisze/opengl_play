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

namespace text_renderer {
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
struct Font_texture {
    using pointer = std::shared_ptr< Font_texture >;
    std::string font_name;
    std::unordered_map<GLchar, character_data> charset;
};

/*
 * Instead of passing everywhere the font name
 * the code will identify the various font by
 * a unique ID
 */
using font_type_id = int;

class Font_texture_loader
{
public:
    using pointer = std::shared_ptr< Font_texture_loader >;
    Font_texture_loader();
    ~Font_texture_loader();
    std::pair< font_type_id, Font_texture::pointer > load_new_textureset( const std::string& font_name );
    Font_texture::pointer get_texture( font_type_id id );
    font_type_id get_default_font_id();
private:
    using glyph_buffer_ptr = std::unique_ptr<GLuint[]>;
    glyph_buffer_ptr create_glyph_buffer( const FT_Face& font_face );
    GLuint create_texture( glyph_buffer_ptr glyph_buffer, const FT_Face& font_face );
    FT_Library freetype_lib;
    font_type_id next_id;
    std::unordered_map<font_type_id, Font_texture::pointer> fonts;
    font_type_id default_font_id;
};

class Renderable_text : public renderer::Renderable
{
public:
    using pointer = std::shared_ptr< Renderable_text >;
    Renderable_text();
    ~Renderable_text();
    Renderable_text( const std::string& text,
                     const glm::vec3& position,
                     GLfloat scale,
                     glm::vec4 color ); //Use the default font
    void set_window_size( int height, int width );
    void set_text( const std::string& text );
    void set_position( const glm::vec3 position );
    void set_scale( GLfloat scale );
    void set_color( glm::vec4 color );

    void prepare_for_render( ) override;
    bool render( ) override;
    void clean_after_render( ) override;
private:
    GLuint VAO, VBO;

    Font_texture_loader   font_loader;
    Font_texture::pointer font_texture;

    std::string text_string;
    glm::vec3   text_position;
    GLfloat     text_scale;
    glm::mat4   text_projection;

    void init();
    void check_for_errors();
};

}

#endif
