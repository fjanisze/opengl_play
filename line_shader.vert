#version 330 core
layout (location = 0) in vec3 position;

out vec4 vertex_color;

uniform mat4 object_position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection *  view * object_position * model *  vec4(position,1.0);
    vertex_color = vec4(0.0,0.0,0.0,1.0);
}
