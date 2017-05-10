#include "text_rendering.hpp"

namespace text_renderer
{

font_texture_loader::font_texture_loader() :
    next_id{ 1 }
{
    LOG3("Creating the font loader object.");
    FT_Error error = FT_Init_FreeType(&freetype_lib);
    if(error){
        ERR("Unable to load the freetype library, error code: ",
            error);
        throw std::runtime_error("Unable to load the freetype library!");
    }
    else
    {
        LOG3("Freetype lib loaded correctly.");
        //Load the default font which shall have the ID 1
        auto default_font = load_new_textureset("/usr/share/fonts/truetype/freefont/FreeSerif.ttf");
        if(default_font.second == nullptr){
            ERR("Failed to load the default font.");
            throw std::runtime_error("Unable to load the default font!.");
        }
        default_font_id = default_font.first;
    }
}

font_texture_loader::~font_texture_loader()
{
    if(freetype_lib != 0){
        FT_Done_FreeType(freetype_lib);
    }
}

/*
 * Attemp to load a new texture set from the path provided in
 * font_name. Return nullptr if the operations fails
 */

std::pair<font_type_id,font_texture_ptr>
font_texture_loader::load_new_textureset(const std::string &font_name)
{
    LOG3("Attempt to load the font: ",
         font_name);
    font_texture_ptr new_font = nullptr;
    font_type_id     new_font_id = 0;
    FT_Face          font_face;
    FT_Error error = FT_New_Face(freetype_lib,
                                 font_name.c_str(),
                                 0,
                                 &font_face);
    if(error)
    {
        ERR("Failed to load the font: ",
            font_name);
    }
    else
    {
        new_font = std::make_shared<font_texture>();
        new_font->font_name = font_name;
        // Set size to load glyphs as
        FT_Set_Pixel_Sizes(font_face, 0, 48);

        //Make sure that the proper alignment is set
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLenum error = glGetError();
        if( GL_NO_ERROR != error ) {
            WARN1("Running load_new_textureset while OpenGL error set to: ",
                  error);
        }
        // Load first 128 characters of ASCII set
        for (GLubyte current_char = 0; current_char < 128; current_char++)
        {
            // Load character glyph
            if (FT_Load_Char(font_face, current_char,
                              FT_LOAD_RENDER ))
            {
                ERR("Failed to load the Glyph for the requested font!");
                return {0,nullptr};
            }

            // Generate the RGBA information
            auto glyph_buffer = create_glyph_buffer( font_face );
            // Generate texture
            GLuint texture = create_texture( std::move( glyph_buffer ), font_face );

            // Now store character for later use
            character_data character = {
                texture,
                glm::fvec2(
                    font_face->glyph->bitmap.width,
                    font_face->glyph->bitmap.rows),
                glm::fvec2(
                    font_face->glyph->bitmap_left,
                    font_face->glyph->bitmap_top),
                font_face->glyph->advance.x
            };
            new_font->charset.insert({current_char,character});
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(font_face);

        new_font_id = next_id++;
        LOG1("Font ",font_name," loaded properly!");
        fonts.insert({new_font_id,
                      new_font});
    }
    return {new_font_id,
                new_font};
}

/*
 * Create a buffer of RGBA pixels on the base of the
 * information in the FT_Face
 */
font_texture_loader::glyph_buffer_ptr
font_texture_loader::create_glyph_buffer(
        const FT_Face &font_face)
{
    LOG1("Create the RGBA glyph buffer.");
    constexpr std::size_t buf_size{ 2048 };
    auto glyph_buffer = std::make_unique<GLuint[]>( buf_size );
    /*
     * FT_Load_Char generate only a buffer of bytes,
     * one for each glyph with greyscale values, but for
     * our texture we need a RGBA pixel format.
     *
     * For such reason I'm creating a new glyph_buffer
     * of RGBA values that can be used to create the proper
     * texture
     */
    int glyph_buffer_idx{ 0 };
    for( int y{0} ; y < font_face->glyph->bitmap.rows ; ++y )
    {
        for( int x{0} ; x < font_face->glyph->bitmap.width ; ++x )
        {
            int idx = x + y * font_face->glyph->bitmap.width;
            if( glyph_buffer_idx >= buf_size ) {
                ERR("Glyph buffer too small!");
                throw std::runtime_error("Glyph buffer too small!");
            }
            auto data = font_face->glyph->bitmap.buffer[ idx ];
            glyph_buffer[ glyph_buffer_idx ] = (
                        data << 24 |
                        data << 16 |
                        data << 8 |
                        data
                        );
            ++glyph_buffer_idx;
        }
    }
    return glyph_buffer;
}

GLuint font_texture_loader::create_texture(
        font_texture_loader::glyph_buffer_ptr glyph_buffer,
        const FT_Face &font_face)
{
    LOG1("Creating the font texture.");
    GLuint texture;

    // Generate texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA8,
                font_face->glyph->bitmap.width,
                font_face->glyph->bitmap.rows,
                0,
                GL_RGBA,
                GL_UNSIGNED_INT_8_8_8_8, //RGBA, each component 8 bit size
                glyph_buffer.get()
                );

    GLenum error = glGetError();
    if(error != GL_NO_ERROR){
        ERR("glTexImage2D ERROR code: ",
            error);
    }
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAX_LEVEL, 0);

    LOG1("Texture ready, ID:",texture);
    return texture;
}

/*
 * Return the pointe to the texture information for the given font,
 * if those information are not available then try to load the font
 * from its location.
 *
 * font_name is actually a path.
 */
font_texture_ptr font_texture_loader::get_texture(font_type_id id)
{
    auto it = fonts.find(id);
    if(it == fonts.end()){
        ERR("Unable to find the font with id ",id);
        return nullptr;
    }
    return it->second;
}

font_type_id font_texture_loader::get_default_font_id()
{
    return default_font_id;
}

void Renderable_text::init()
{
    LOG3("renderable_text::init: VBO, VBA");
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat)*6*5,
                 NULL,
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid*)0);
    // Vertex Texture Coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid*)(sizeof(GLfloat)*3));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    check_for_errors();

    //Use default font textures
    font_texture = font_loader.get_texture(
                font_loader.get_default_font_id()
                );
    light_calc_uniform = -1;
}

void Renderable_text::check_for_errors()
{
    int error_count = 0;
    GLenum error = GL_NO_ERROR;

    while( (error = glGetError()) != GL_NO_ERROR){
        ERR("OpenGL ERROR: ",
            error);
        ++error_count;
    }

    if(error_count>0){
        throw std::runtime_error("check_for_errors: ERRORS!!");
    }
}

Renderable_text::Renderable_text()
{
    init();
}

Renderable_text::Renderable_text(const std::string &text,
                                 const glm::vec3& position,
                                 GLfloat scale,
                                 glm::vec4 color) :
    text_position{ position },
    text_scale{ scale },
    text_string{ text }
{
    set_position( position );
    set_color( color );
    init();
}

void Renderable_text::set_window_size(int height,
                                      int width)
{
}

void Renderable_text::set_text(const std::string &text)
{
    text_string = text;
}

void Renderable_text::set_position(const glm::vec3 position)
{
    text_position = position;
    if( view_configuration.is_world_space() ) {
        rendering_data.model_matrix = glm::translate( rendering_data.model_matrix,
                                       position );
    }
}

void Renderable_text::set_scale(GLfloat scale)
{
    text_scale = scale;
    for( auto&& ch : font_texture->charset ) {
        ch.second.Bearing *= scale;
        ch.second.Size *= scale;
        ch.second.Advance *= scale;
    }
}

void Renderable_text::set_color(glm::vec4 color)
{
    LOG1("Setting text color to: ", color);
    rendering_data.default_color = color;
}

void Renderable_text::prepare_for_render( shaders::shader_ptr &shader )
{
    shader->disable_light_calculations();
}

void Renderable_text::render( shaders::shader_ptr &shader )
{
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    GLfloat x{ 0.0f },
            y{ 0.0f };
    if( view_configuration.is_camera_space() ) {
        x = text_position.x;
        y = text_position.y;
    }
    for (c = text_string.begin(); c != text_string.end(); c++)
    {
        character_data ch = font_texture->charset[*c];

        GLfloat xpos = x + ch.Bearing.x;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y);

        GLfloat w = ch.Size.x;
        GLfloat h = ch.Size.y;
        // Update VBO for each character

        GLfloat vertices[6][5] = {
            { xpos + w, ypos + h,  0.0 ,   1.0, 0.0 },
            { xpos + w, ypos,      0.0 ,   1.0, 1.0 },
            { xpos,     ypos + h,  0.0 ,   0.0, 0.0 },

            { xpos + w, ypos,      0.0 ,   1.0, 1.0 },
            { xpos,     ypos,      0.0 ,   0.0, 1.0 },
            { xpos,     ypos + h,  0.0 ,   0.0, 0.0 },
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        sizeof(vertices),
                        vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance / 64.0f);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void Renderable_text::clean_after_render( shaders::shader_ptr &shader )
{
    shader->enable_light_calculations();
}


}
