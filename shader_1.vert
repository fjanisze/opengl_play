#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;
layout (location = 2) in vec3 normal_vec;

out vec2 texture_coords;
out vec3 normal;
out vec3 frag_pos;
out vec3 camera_pos;

uniform mat4 object_position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    normal = mat3(transpose(inverse(model))) * normal_vec;
    texture_coords = vec2(tex_coord.x,tex_coord.y);
    frag_pos = vec3( model * vec4(position, 1.0f) );
    camera_pos = inverse(view)[3].xyz;
}
