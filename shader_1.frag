#version 330 core
in vec2 texture_coords;
in vec3 normal;
in vec3 frag_pos;
in vec3 camera_pos;

out vec4 color;

uniform sampler2D loaded_texture;
uniform sampler2D loaded_texture_specular_map;
uniform vec3      object_color;
uniform int       number_of_lights;
uniform float     light_data[ 1024 ];

void main()
{
    vec3 final_object_color;
    //Calculate lights
    vec3 diffuse_res = vec3(0.0);
    vec3 spec_res = vec3(0.0);
    float attenuation;
    vec3 light_dir;
    vec3 norm = normalize(normal);
    int buf_idx = 0;
    float light_type;
    vec3 light_pos,
         light_color;
    float light_strength;
    for( int light_idx = 0 ; light_idx < number_of_lights ; ++light_idx ) {
	/*
	 * Extract the light data from the common light buffer
	 */
	if( light_data[ buf_idx ] <= 1 ) //Point light or directional light
	{
	    light_type = light_data[ buf_idx ];
	    ++buf_idx;
	    light_pos = vec3( light_data[ buf_idx ],
	            light_data[ buf_idx + 1],
	            light_data[ buf_idx + 2]);
	    buf_idx += 3;
	    light_color = vec3( light_data[ buf_idx],
	            light_data[ buf_idx + 1],
	            light_data[ buf_idx + 2]);
	    buf_idx += 3;
	    light_strength = light_data[ buf_idx ];
	    ++buf_idx;
	}
	//Now perform the calculations
	if( light_type == 0 ) //Point light
	{
	    float dist = length(light_pos - frag_pos);
	    attenuation = (1.0 + dist * 0.22 + dist*dist*0.2);
	    attenuation = light_strength / attenuation;
	    light_dir = normalize( light_pos - frag_pos );
	}
	else if( light_type == 1) //Directional light
	{
	    light_dir = normalize( light_pos );
	    attenuation = 1.0;
	}
	//Common diffuse light calculations
	float diff = max( dot( norm, light_dir ), 0.0);
	vec3 diffuse = diff * light_color;
	diffuse *= ( vec3(texture(loaded_texture,texture_coords)) * attenuation );
	diffuse_res += diffuse;
	//The specular calculations are the same for both the lights
	vec3 view_dir = normalize( camera_pos - frag_pos );
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir,reflect_dir),0.0),32);
	vec3 specular =  spec * light_color;
	specular *= (vec3(texture(loaded_texture_specular_map,texture_coords)) * .5 * attenuation);
	spec_res += specular;
    }
    final_object_color = ( diffuse_res + spec_res ) * object_color;
    //Final color
    color = vec4( final_object_color ,1.0);
}
