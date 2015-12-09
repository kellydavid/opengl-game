#include <GLUT/GLUT.h>
#include <OpenGl/gl3.h>
#include <iostream>
#include <math.h>
#include "maths_funcs.h"
#include "shaders.hpp"
#include "utilities.hpp"
#include "model.hpp"
#include "game_object.hpp"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

int number_of_models = NUMBER_MODELS;
string *model_filenames = new string[number_of_models]{VEHICLE_MODEL, STREET_MODEL};

int width = 800;
int height = 800;

CameraTransform camera_transform;
ModelTransform vehicle_transform;

ShaderProgram programs[SH_NUM_PROGRAM_TYPES];
vector<GameObject> gameObjects(NUMBER_MODELS);

void display(){
    
    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for(int i = 0; i < SH_NUM_PROGRAM_TYPES; i++){
        glUseProgram(programs[i].programID);
        
        //Declare your uniform variables that will be used in your shaders
        int view_mat_location = glGetUniformLocation (programs[i].programID, SH_UNIFORM_VIEW);
        int proj_mat_location = glGetUniformLocation (programs[i].programID, SH_UNIFORM_PERSPECTIVE);
        
        // values of matrices
        mat4 view = identity_mat4();
        view = look_at(camera_transform.eye, camera_transform.look(), vec3(0.0, 1.0, 0.0));
        mat4 persp_proj = perspective(90.0, (float)width/(float)height, 0.1, 10000.0);
        
        // update uniforms & draw
        glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
        glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view.m);
    }
    
    for(int i = 0; i < NUMBER_MODELS; i++){
        gameObjects[i].draw_model(programs);
    }
    
    glutSwapBuffers();
}

void updateScene() {
    
    // Placeholder code, if you want to work with framerate
    // Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
    /*
     static DWORD  last_time = 0;
     DWORD  curr_time = timeGetTime();
     float  delta = (curr_time - last_time) * 0.001f;
     if (delta > 0.03f)
     delta = 0.03f;
     last_time = curr_time;
     */
    // Draw the next frame
    glutPostRedisplay();
}

void initialise_transforms(){
    camera_transform.eye = vec3(0.0, 10.0, 0.0);
    gameObjects[1].modelTransform.scale = vec3(6.0, 6.0, 6.0);
    gameObjects[0].modelTransform.scale = vec3(0.8, 0.8, 0.8);
    gameObjects[0].modelTransform.rotation = vec3(-90.0, 0.0, 0.0);
    gameObjects[0].modelTransform.translation = vec3(-90.0, 0.0, 180.0);
}

void init()
{
    // setup vaos
    for(int i = 0; i < NUMBER_MODELS; i++){
        gameObjects[i].setup_vao();
    }
    
    for(int i = 0; i < SH_NUM_PROGRAM_TYPES; i++){
        programs[i] = *new ShaderProgram(static_cast<SH_PROGRAM_TYPE>(i));
    }
    
    for(int i = 0; i < NUMBER_MODELS; i++){
        gameObjects[i].load_model(model_filenames[i]);
    }
    
    initialise_transforms();
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    bool turn_right, turn_left, up, down, forward, back;
    if(key == 'a'){
        turn_left = true;
    }else{
        turn_left = false;
    }
    
    if(key == 'd'){
        turn_right = true;
        
    }else{
        turn_right = false;
    }
    
    if(key == 'w'){
        forward = true;
    }else{
        forward = false;
    }
    
    if(key == 's'){
        back = true;
    }else{
        back = false;
    }
    
    if(key == 'e'){
        up = true;
    }else{
        up = false;
    }
    
    if(key == 'q'){
        down = true;
    }else{
        down = false;
    }
    
    if(turn_left){
        camera_transform.angle -= camera_transform.angle_inc;
        camera_transform.lx = sin(camera_transform.angle);
        camera_transform.lz = -cos(camera_transform.angle);
    }
    if(turn_right){
        camera_transform.angle += camera_transform.angle_inc;
        camera_transform.lx = sin(camera_transform.angle);
        camera_transform.lz = -cos(camera_transform.angle);
    }
    if(forward){
        camera_transform.eye.v[0] += camera_transform.lx * camera_transform.fraction;
        camera_transform.eye.v[2] += camera_transform.lz * camera_transform.fraction;
    }
    if(back){
        camera_transform.eye.v[0] -= camera_transform.lx * camera_transform.fraction;
        camera_transform.eye.v[2] -= camera_transform.lz * camera_transform.fraction;
    }
    if(up){
        camera_transform.eye.v[1] += camera_transform.fraction;
    }
    if(down){
        camera_transform.eye.v[1] -= camera_transform.fraction;
    }
}

int main(int argc, char** argv){
    
    // Set up the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Street Vehicle");
    
    // Tell glut where the display function is
    glutDisplayFunc(display);
    glutIdleFunc(updateScene);
    glutKeyboardFunc(keypress);
    
    // Set up your objects and shaders
    init();
    // Begin infinite event loop
    glutMainLoop();
    return 0;
}











