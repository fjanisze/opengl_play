#ifndef TYPES_HPP
#define TYPES_HPP

#include <utility>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>

namespace types
{
//Source and direction
using ray_t = std::pair<glm::vec3,glm::vec3>;

using color = glm::vec4; //RGBA

using point = glm::vec3;
using vector = glm::vec3;

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

using key_code_t = int;
using scan_code_t = int;
using act_code_t = int;

using id_type = uint64_t;

using duration = std::chrono::duration< double >;
using time_point = std::chrono::time_point< std::chrono::high_resolution_clock, duration >;
using timestamp = std::chrono::microseconds;

}

#endif
