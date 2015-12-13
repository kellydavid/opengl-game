#version 330

in vec3 texture_coordinates;

uniform samplerCube cube_texture;

out vec4 FragColor;

void main () {
    FragColor = texture(cube_texture, texture_coordinates);
}