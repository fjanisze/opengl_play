#include <framebuffers.hpp>
#include <logger/logger.hpp>

namespace buffers
{

Framebuffers::Framebuffers( const types::win_size& window ) :
    window_size{ window }
{
    LOG1("Creating a new framebuffers, size: ",
         window.width,"/",window.height);
}

GLuint Framebuffers::create_buffer()
{
    LOG3("Creating a new buffer!");

    Buffer new_buf;
    glGenFramebuffers(1, &new_buf.FBO);
    glBindFramebuffer( GL_FRAMEBUFFER,
                       new_buf.FBO );

    //Create and attach a texture
    new_buf.texture = create_texture();
    glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                new_buf.texture,
                0);
    /*
     *Create and attach a renderbuffer for stencil
     *and depth testing.
     */
    new_buf.RBO = create_renderbuffer();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              new_buf.RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if( false == buffers.insert( new_buf ).second ) {
        ERR("Failed while inserting a new FBO with ID ", new_buf.FBO);
        glDeleteTextures( 1, &new_buf.texture );
        return -1;
    }

    LOG3("Creation completed, all buffers ready!");
    return new_buf.FBO;
}

GLenum Framebuffers::bind(const GLuint fbo)
{
    if( buffers.find( Buffer(fbo) ) == buffers.end() ) {
        ERR("Buffer ",fbo," not found!");
        return GL_INVALID_OPERATION;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum buffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if( GL_FRAMEBUFFER_COMPLETE != buffer_status ) {
        ERR("Cannot bind the framebuffer, status: ",
            buffer_status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return buffer_status;
}

void Framebuffers::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffers::~Framebuffers()
{
    for( auto& fbo : buffers ) {
        glDeleteBuffers( 1, &fbo.FBO );
    }
}

long Framebuffers::clear(GLuint buffer_id)
{
    auto clear_op = []( ) {
        glClearColor(0.0,0.0,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    };

    if( 0 != buffer_id ) {
        bind( buffer_id );
        clear_op();
    } else {
        /*
         * First clear the default buffer,
         * then all the framebuffers.
         */
        unbind(); //Make sure the def buffer is binded
        clear_op();
        for( const auto& id : buffers ) {
            bind( id );
            clear_op();
        }
    }
    unbind();
}

GLuint Framebuffers::create_renderbuffer()
{
    LOG3("Creating new renderbuffer");
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,
                       rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          window_size.width,
                          window_size.height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return rbo;
}

GLuint Framebuffers::create_texture()
{
    LOG3("Creating a new texture, size: ",
         window_size.width,"/",window_size.height);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 window_size.width,
                 window_size.height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    LOG3("New texture ID: ", texture);
    return texture;
}

}
