#include <textures.hpp>
#include <logger/logger.hpp>
#include <mutex>

namespace textures
{

typedef unsigned char byte_t;

texture_t load_texture(const std::string &filename,
                       GLint wrapping_method)
{
    LOG1("load_texture: ",filename.c_str());
    texture_t TEX;
    //Load the texture image
    byte_t* image = SOIL_load_image(filename.c_str(),
                                    &TEX.width,
                                    &TEX.height,
                                    0,
                                    SOIL_LOAD_RGB);
    if(image == nullptr){
        ERR("Unable to load the texture! ",
            filename.c_str());
        throw std::runtime_error("Texture loading failed!");
    }
    LOG1("Loaded texture size: ",TEX.width,"/",TEX.height);
    //Create the texture
    glGenTextures(1,&TEX.id);
    glBindTexture(GL_TEXTURE_2D,TEX.id);

    //Texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, wrapping_method);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, wrapping_method);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,	0,
                 GL_RGB,
                 TEX.width,
                 TEX.height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 image);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D,0);

    return TEX;
}

}
