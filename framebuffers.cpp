#include <framebuffers.hpp>
#include <logger/logger.hpp>

namespace Framebuffers
{

framebuffers::framebuffers(GLuint screen_width,
                           GLuint screen_height) :
    width{ screen_width },
    height{ screen_height }
{
    LOG1("Creating a new framebuffers, size: ",
         screen_width,"/",screen_height);
}

GLuint framebuffers::create_buffer()
{
    LOG3("Creating a new buffer!");

    buffer new_buf;
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

GLenum framebuffers::bind(const GLuint fbo)
{
    if( buffers.find( buffer(fbo) ) == buffers.end() ) {
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

void framebuffers::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

framebuffers::~framebuffers()
{
    for( auto& fbo : buffers ) {
        glDeleteBuffers( 1, &fbo.FBO );
    }
}

GLuint framebuffers::create_renderbuffer()
{
    LOG1("Creating new renderbuffer");
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER,
                       rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH24_STENCIL8,
                          width,
                          height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return rbo;
}

GLuint framebuffers::create_texture()
{
    LOG1("Creating a new texture, size: ",
         width,"/",height);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 width,
                 height,
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