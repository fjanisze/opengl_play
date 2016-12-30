#version 330 core
in vec3 out_color;
in vec2 texture_coords;

out vec4 color;

uniform sampler2D loaded_texture;
uniform sampler2D loaded_texture_2;
uniform vec3      object_color;
uniform vec3      light_color;
uniform float mix_ratio = 0.2;

void main()
{
    color = vec4( object_color * light_color, 1.0) *
            mix( texture(loaded_texture, texture_coords),
                 texture(loaded_texture_2, texture_coords), mix_ratio);
}
