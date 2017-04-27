#ifndef TYPES_HPP
#define TYPES_HPP

#include <utility>
#include <glm/gtc/type_ptr.hpp>

namespace types
{
//Source and direction
using ray_t = std::pair<glm::vec3,glm::vec3>;

using color = glm::vec4; //RGBA

namespace internal
{

struct window_size
{
    const uint64_t width;
    const uint64_t height;

    window_size() = default;
    window_size( const uint64_t width,
                 const uint64_t height) :
        width{ width },
        height{ height }
    {}
};

}

using win_size = internal::window_size;
}

#endif
