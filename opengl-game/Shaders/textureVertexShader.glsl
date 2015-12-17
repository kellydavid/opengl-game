#version 330

layout (location = 0)in vec3 vertex_position;
layout (location = 1)in vec3 vertex_normal;
layout (location = 2)in vec2 vertex_texture;

out vec2 texture_coordinates;
out vec3 LightIntensity;
out vec3 position_eye, normal_eye;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out float visibility;
const float density = 0.001;
const float gradient = 2.0;

void main(){
/*
  mat4 ModelViewMatrix = view * model;
  mat3 NormalMatrix =  mat3(ModelViewMatrix);
  // Convert normal and position to eye coords
  // Normal in view space
  vec3 tnorm = normalize( NormalMatrix * vertex_normal);
  // Position in view space
  vec4 eyeCoords = ModelViewMatrix * vec4(vertex_position,1.0);
  //normalised vector towards the light source
 vec3 s = normalize(vec3(LightPosition - eyeCoords));
  
  // The diffuse shading equation, dot product gives us the cosine of angle between the vectors
  LightIntensity = Ld * Kd * max( dot( s, tnorm ), 0.0 );
  
  // Convert position to clip coordinates and pass along
  gl_Position = proj * view * model * vec4(vertex_position,1.0);
    
 */
    vec4 world_position = model * vec4(vertex_position,1.0);
    vec4 positionRelativeToCam = view * world_position;
    float distance = length(positionRelativeToCam.xyz);
    visibility = exp(-pow((distance * density),gradient));
    
    texture_coordinates = vertex_texture; // output texture coordinates
    position_eye = vec3 (view * model * vec4 (vertex_position, 1.0));
    normal_eye = vec3 (view * model * vec4 (vertex_normal, 0.0));
    gl_Position = proj * vec4 (position_eye, 1.0);
}


  