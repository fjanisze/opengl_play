#version 330 core
in vec2 texture_coords;
in vec3 normal;
in vec3 frag_pos;
in vec3 camera_pos;

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
    vec3 final_object_color;
    //Calculate lights
    vec3 diffuse_res = vec3(0.0);
    vec3 spec_res = vec3(0.0);
    for( int i = 0 ; i < 2 ; ++i ) {
	vec3 norm = normalize(normal);
	float light_intensity = sqrt(length(light_pos[i] - frag_pos));
	vec3 light_dir = normalize( light_pos[i] - frag_pos );
	float diff = max( dot( norm, light_dir), 0.0) / light_intensity;
	diffuse_res += diff * light_color[i];

	vec3 view_dir = normalize( camera_pos - frag_pos );
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir,reflect_dir),0.0),32);
	spec_res += spec * light_color[i] * .5;
    }
    final_object_color = ( diffuse_res + spec_res + ambient ) * 2 * object_color;
    //Final color
    color = vec4( final_object_color ,1.0) *
            vec4( mix( texture(loaded_texture, texture_coords),
                 texture(loaded_texture_2, texture_coords), mix_ratio) );
}
