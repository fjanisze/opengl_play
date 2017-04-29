#ifndef FRAMEBUFFERS_HPP
#define FRAMEBUFFERS_HPP
#include <headers.hpp>
#include <set>
#include <types.hpp>

namespace buffers
{
/*
 * Internal rappresentation
 * of a framebuffer
 */
struct Buffer
{
    Buffer() = default;
    Buffer( const GLuint fbo ) :
        FBO{ fbo } {}

    GLuint FBO{ GL_INVALID_INDEX };
    GLuint texture{ GL_INVALID_INDEX };
    GLuint RBO{ GL_INVALID_INDEX };

    operator GLuint() const {
        return FBO;
    }

    bool operator < ( const Buffer& other ) const {
        return FBO < other.FBO;
    }
};

/*
 * Handle the creation and destruction
 * of framebuffers
 */
class Framebuffers
{
public:
    using pointer = std::shared_ptr< Framebuffers >;
    using buffer_id_t = GLuint;

    Framebuffers( const types::win_size& window );
    GLuint create_buffer();
    GLenum bind( const GLuint fbo );
    void unbind();
    ~Framebuffers();
    /*
     * Clear all the buffers or just the
     * selected one.
     */
    long clear( GLuint buffer_id = 0 );
private:
    GLuint create_renderbuffer();
    GLuint create_texture();
    std::set<Buffer> buffers;
    types::win_size window_size;
};

}

#endif //FRAMEBUFFERS_HPP
