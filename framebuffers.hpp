#ifndef FRAMEBUFFERS_HPP
#define FRAMEBUFFERS_HPP
#include <headers.hpp>
#include <set>

namespace Framebuffers
{
/*
 * Internal rappresentation
 * of a framebuffer
 */
struct buffer
{
    buffer() = default;
    buffer( const GLuint fbo ) :
        FBO{ fbo } {}

    GLuint FBO{ GL_INVALID_INDEX };
    GLuint texture{ GL_INVALID_INDEX };
    GLuint RBO{ GL_INVALID_INDEX };

    operator GLuint() const {
        return FBO;
    }

    bool operator < ( const buffer& other ) const {
        return FBO < other.FBO;
    }
};

/*
 * Handle the creation and destruction
 * of framebuffers
 */
class framebuffers;
using framebuffers_ptr = std::shared_ptr< framebuffers >;
class framebuffers
{
public:
    framebuffers( GLuint screen_width,
                  GLuint screen_height );
    GLuint create_buffer();
    GLenum bind( const GLuint fbo );
    void unbind();
    ~framebuffers();

    static framebuffers_ptr create( const GLuint screen_w,
                                    const GLuint screen_h ) {
        return std::make_shared< framebuffers >( screen_w, screen_h );
    }

private:
    GLuint create_renderbuffer();
    GLuint create_texture();
    std::set<buffer> buffers;
    GLuint width;
    GLuint height;
};

}

#endif //FRAMEBUFFERS_HPP
