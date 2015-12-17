#version 330

in vec3 LightIntensity;

in vec3 position_eye;
in vec3 normal_eye;

in vec2 texture_coordinates;
uniform sampler2D basic_texture;

uniform mat4 view;

uniform vec3 world_light_position;

in float visibility;
vec3 skyColour = vec3(0.8, 0.8, 0.8);

// Fixed point light
vec3 Ls = vec3 (0.8, 0.8, 0.8); // white specular colour
vec3 Ld = vec3 (0.6, 0.6, 0.6); // dull white diffuse light colour
vec3 La = vec3 (0.4, 0.4, 0.4); // grey ambient colour

// Surface reflectance
vec3 Ks = vec3 (0.2, 0.2, 0.2); // fully reflect specular light
vec3 Kd = vec3 (1.0, 1.0, 1.0); // orange diffuse surface reflectance
vec3 Ka = vec3 (0.3, 0.3, 0.3); // fully reflect ambient light
float specular_exponent = 1.0; // specular 'power'

out vec4 FragColor;

void main(){
    // ambient intensity
    vec3 Ia = La * Ka;
    
    // diffuse intensity
    // raise light position to eye space
    vec3 light_position_eye = vec3 (view * vec4 (world_light_position, 1.0));
    vec3 distance_to_light_eye = light_position_eye - position_eye;
    vec3 direction_to_light_eye = normalize (distance_to_light_eye);
    float dot_prod = dot (direction_to_light_eye, normal_eye);
    dot_prod = max (dot_prod, 0.0);
    vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity
    
    // specular intensity
    vec3 surface_to_viewer_eye = normalize (-position_eye);
    
    //vec3 reflection_eye = reflect (-direction_to_light_eye, normal_eye);
    //float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
    //dot_prod_specular = max (dot_prod_specular, 0.0);
    //float specular_factor = pow (dot_prod_specular, specular_exponent);
    
    // blinn
    vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
    float dot_prod_specular = max (dot (half_way_eye, normal_eye), 0.0);
    float specular_factor = pow (dot_prod_specular, specular_exponent);
    
    vec3 Is = Ls * Ks * specular_factor; // final specular intensity
    
    // final colour
	FragColor = vec4 (Is + Id + Ia, 1.0);
    
    
    vec4 texel = texture(basic_texture, texture_coordinates);
    FragColor = texel * FragColor;
    FragColor = mix(vec4(skyColour, 1.0), FragColor, visibility);
}