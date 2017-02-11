#version 330 core

uniform vec3 light_color;
uniform float light_strength;
out vec4 color;

void main()
{
    color = vec4(light_color * light_strength, 1.0);
}
