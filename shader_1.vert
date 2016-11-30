#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 tex_coord;

out vec3 out_color;
out vec2 texture_coords;

uniform mat4 object_position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * object_position* model * vec4(position, 1.0);
    out_color = color;
    texture_coords = vec2(tex_coord.x,tex_coord.y);
}
