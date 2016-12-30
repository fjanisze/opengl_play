#version 330 core
in vec3 out_color;
in vec2 texture_coords;

out vec4 color;

uniform sampler2D loaded_texture;
uniform sampler2D loaded_texture_2;
uniform vec3      object_color;
uniform vec3      ambient_light_color;
uniform float     mix_ratio = 0.2;
uniform float     ambient_light_strenght = 1.0;

void main()
{
    vec3 ambient = ambient_light_strenght * ambient_light_color;
    color = vec4( mix( ambient , object_color, 0.01),1.0) *
            vec4( mix( texture(loaded_texture, texture_coords),
                 texture(loaded_texture_2, texture_coords), mix_ratio) );
}
