#version 330 core
in vec2 texture_coords;
in vec3 normal;
in vec3 frag_pos;
in vec3 camera_pos;

out vec4 color;

uniform sampler2D loaded_texture1;
uniform sampler2D loaded_texture_specular_map1;

uniform sampler2D loaded_texture2;
uniform sampler2D loaded_texture_specular_map2;

uniform sampler2D loaded_texture3;
uniform sampler2D loaded_texture_specular_map3;

uniform vec4      object_color;
uniform int       number_of_lights;
uniform float     light_data[ 800 ];
uniform bool      skip_light_calculations;
uniform bool      skip_texture_calculations;

vec4 calculate_lighting( vec4 tex )
{
    vec4 diffuse_res = vec4(0.0);
    vec4 spec_res = vec4(0.0);
    float attenuation = 1.0;
    vec3 light_dir;
    vec3 norm = normalize(normal);
    int buf_idx = 0;
    float light_type;
    vec3 light_pos,
         light_direction;
    vec4 light_color;
    float light_strength,
          cut_off_angle,
          out_cutoff_angle;
    for( int light_idx = 0 ; light_idx < number_of_lights ; ++light_idx ) {
	/*
	 * Extract the light data from the common light buffer.
	 * The initial set of informations is common to all the lights
	 */
	if( light_data[ buf_idx ] <= 3 )
	{
	    light_type = light_data[ buf_idx ];
	    ++buf_idx;
	    light_pos = vec3( light_data[ buf_idx ],
	            light_data[ buf_idx + 1],
	            light_data[ buf_idx + 2]);
	    buf_idx += 3;
	    light_color = vec4( light_data[ buf_idx ],
	            light_data[ buf_idx + 1],
	            light_data[ buf_idx + 2],
	            light_data[ buf_idx + 3]);
	    buf_idx += 4;
	    light_strength = light_data[ buf_idx ];
	    ++buf_idx;
	}
	//Extract specific information
	if( light_type == 2 || light_type == 3)
	{
	    light_direction = vec3( light_data[ buf_idx ],
	            light_data[ buf_idx + 1],
	            light_data[ buf_idx + 2]);
	    buf_idx += 3;
	    cut_off_angle = light_data[ buf_idx++ ];
	    out_cutoff_angle = light_data[ buf_idx ];
	    ++buf_idx;
	}
	//Now perform the calculations
	if( light_type == 0 ) //Point light
	{
	    float dist = length( light_pos - frag_pos );
	    attenuation = (1.0 + dist * 0.22 + dist*dist*0.2);
	    attenuation = max( light_strength / attenuation, .5);
	    light_dir = normalize( light_pos - frag_pos );
	}
	else if( light_type == 1 ) //Directional light
	{
	    float dist = length( light_pos - frag_pos );
	    light_dir = normalize( light_pos );
	    attenuation = min( light_strength / sqrt( dist ), 1 );
	}
	else if( light_type == 2 || light_type == 3) //Spot light or flashlight
	{
	    light_dir = normalize( light_pos - frag_pos );
	    float dist = length( light_pos - frag_pos );
	    //This angle define the cone of the light,
	    //everything whithin this code is illuminated
	    float theta = dot( light_dir, normalize(-light_direction) );
	    //Angle which define the outer cone of the light,
	    //Things moving away from the theta angle are less
	    //illuminated
	    float epsilon = cut_off_angle - out_cutoff_angle;
	    float intensity = clamp((theta - out_cutoff_angle) / epsilon,
	                            0.0,1.0);
	    if( light_type == 2 )
		attenuation = 1.0 + dist * 0.20;
	    else
		attenuation = 1.0 + dist * 0.2;
	    attenuation = min( light_strength * intensity / attenuation, 1 );
	}
	if( attenuation == 0 ) {
	    continue;
	}
	//Common diffuse light calculations
	float diff = max( dot( norm, light_dir ), 0.4);
	vec4 diffuse = vec4(diff * light_color.rgb, light_color.a);
	if( false == skip_texture_calculations )
	{
	    diffuse *= tex;
	}
	diffuse *= attenuation;
	diffuse_res += diffuse;
	//The specular calculations are the same for both the lights
	vec3 view_dir = normalize( camera_pos - frag_pos );
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow( max( dot(view_dir,reflect_dir), 0.0), 32);
	vec4 specular = vec4(spec * light_color.rgb, light_color.a);
	if( false == skip_texture_calculations )
	{
	    specular *= (texture(loaded_texture_specular_map1,texture_coords) * .3 * attenuation);
	}
	spec_res += specular;
    }
    //Final color
    return ( diffuse_res + spec_res );
}

void main()
{
    vec4 tex_result = vec4(1.0f);
    if( false == skip_texture_calculations )
    {
	tex_result = texture(loaded_texture1,texture_coords);
    }
    if( false == skip_light_calculations ) {
	tex_result = calculate_lighting( tex_result );
    }
    //Final color
    color = tex_result * object_color;
}
