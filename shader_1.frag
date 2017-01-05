#version 330 core
in vec2 texture_coords;
in vec3 normal;
in vec3 frag_pos;

out vec4 color;

uniform sampler2D loaded_texture;
uniform sampler2D loaded_texture_2;
uniform vec3      object_color;
uniform vec3      ambient_light_color;
uniform vec3      light_pos[2]; //Let's support only two lights
uniform vec3      light_color[2];
uniform float     mix_ratio = 0.2;
uniform float     ambient_light_strenght = 1.0;

void main()
{
    vec3 ambient = ambient_light_strenght * ambient_light_color;
    //Calculate diffuse lighting
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize( light_pos[0] - frag_pos );
    float diff = max( dot( norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color[0];
    vec3 final_object_color = (diffuse + ambient) * object_color;
    //Final color
    color = vec4( final_object_color ,1.0) *
            vec4( mix( texture(loaded_texture, texture_coords),
                 texture(loaded_texture_2, texture_coords), mix_ratio) );
}
