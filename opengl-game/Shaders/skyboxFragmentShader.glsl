#version 330

in vec3 position_eye;
in vec3 normal_eye;

uniform samplerCube cube_texture;
uniform mat4 view; // view matrix

out vec4 FragColor;

void main () {
    /* reflect ray around normal from eye to surface */
    vec3 incident_eye = normalize (position_eye);
    vec3 normal = normalize (normal_eye);
    
    vec3 reflected = reflect (incident_eye, normal);
    // convert from eye to world space
    reflected = vec3 (inverse (view) * vec4 (reflected, 0.0));
    
    FragColor = texture (cube_texture, reflected);
}