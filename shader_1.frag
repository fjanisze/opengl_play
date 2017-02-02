#version 330 core
in vec2 texture_coords;
in vec3 normal;
in vec3 frag_pos;
in vec3 camera_pos;

out vec4 color;

uniform sampler2D loaded_texture;
uniform sampler2D loaded_texture_specular_map;
uniform vec3      object_color;
uniform vec3      ambient_light_color;
uniform int       number_of_lights; // Should be <=10
uniform int       light_type[ 10 ];
uniform vec3      light_pos[ 10 ];
uniform vec3      light_color[ 10 ];
uniform float     light_strength[ 10 ];
uniform float     ambient_light_strenght = 1.0;

void calculate_spot_light(inout vec3 diffuse_res,
                          inout vec3 spec_res,
                          int light_idx)
{
    vec3 norm = normalize(normal);
    float dist = length(light_pos[light_idx] - frag_pos);
    float attenuation = (1.0 + dist * 0.22 + dist*dist*0.2);
    attenuation = light_strength[light_idx] / attenuation;
    vec3 light_dir = normalize( light_pos[light_idx] - frag_pos );
    float diff = max( dot( norm, light_dir ), 0.0);
    vec3 diffuse = diff * light_color[light_idx];
    diffuse *= ( vec3(texture(loaded_texture,texture_coords)) * attenuation );
    diffuse_res += diffuse;

    vec3 view_dir = normalize( camera_pos - frag_pos );
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir,reflect_dir),0.0),32);
    vec3 specular =  spec * light_color[light_idx];
    specular *= (vec3(texture(loaded_texture_specular_map,texture_coords)) * .5 * attenuation);
    spec_res += specular;
}

void main()
{
    vec3 ambient = ambient_light_strenght * ambient_light_color;
    vec3 final_object_color;
    //Calculate lights
    vec3 diffuse_res = vec3(0.0);
    vec3 spec_res = vec3(0.0);
    for( int i = 0 ; i < number_of_lights ; ++i ) {
	//Spot_Light
	if( light_type[ i ] == 0 ) {
	    calculate_spot_light(diffuse_res,spec_res,i);
	}
    }
    final_object_color = ( diffuse_res + spec_res + ambient ) * object_color;
    //Final color
    color = vec4( final_object_color ,1.0);
}
