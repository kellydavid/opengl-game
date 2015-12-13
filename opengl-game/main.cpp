#include <GLUT/GLUT.h>
#include <OpenGl/gl3.h>
#include <iostream>
#include <math.h>
#include "maths_funcs.h"
#include "shaders.hpp"
#include "transform.hpp"
#include "model.hpp"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

int width = 800;
int height = 800;

CameraTransform camera_transform;
ModelTransform vehicle_transform;

bool keystates[256];

ShaderProgram programs[SH_NUM_PROGRAM_TYPES];

string *model_filenames = new string[NUMBER_MODELS]{VEHICLE_MODEL, STREET_MODEL, SKYBOX_MODEL};
vector<Model> models(NUMBER_MODELS);

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
        models[i].draw_model(programs);
    }
    
    glutSwapBuffers();
}

void UpdateKeys(){
    bool turn_right = false, turn_left = false, up = false, down = false, forward = false, back = false;
    bool p_right = false, p_left = false, p_forward = false, p_backward = false;
    
    if(keystates['a']){
        turn_left = true;
    }
    if(keystates['d']){
        turn_right = true;
    }
    if(keystates['w']){
        forward = true;
    }
    if(keystates['s']){
        back = true;
    }
    if(keystates['e']){
        up = true;
    }
    if(keystates['q']){
        down = true;
    }
    
    if(keystates['i']){
        p_forward = true;
    }
    if(keystates['k']){
        p_backward = true;
    }
    if(keystates['j']){
        p_left = true;
    }
    if(keystates['l']){
        p_right = true;
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
    
    float inc = 0.5;
    float angle = 5.0;
    
    if(p_forward){
        float radians_angle = models[0].modelTransform.rotation.v[1] * (M_PI / 180);
        models[0].modelTransform.translation.v[0] += sin(radians_angle) * inc;
        models[0].modelTransform.translation.v[2] += cos(radians_angle) * inc;
    }
    if(p_backward){
        float radians_angle = models[0].modelTransform.rotation.v[1] * (M_PI / 180);
        models[0].modelTransform.translation.v[0] -= sin(radians_angle) * inc;
        models[0].modelTransform.translation.v[2] -= cos(radians_angle) * inc;
    }
    if(p_left){
        models[0].modelTransform.rotation.v[1] += angle;
    }
    if(p_right){
        models[0].modelTransform.rotation.v[1] -= angle;
    }
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
    UpdateKeys();
    glutPostRedisplay();
}

void initialise_transforms(){
    camera_transform.eye = vec3(0.0, 10.0, 0.0);
    models[SKYBOX_INDEX].modelTransform.scale = vec3(800.0, 800.0, 800.0);
    models[STREET_INDEX].modelTransform.scale = vec3(6.0, 6.0, 6.0);
    models[VEHICLE_INDEX].modelTransform.scale = vec3(20.0, 20.0, 20.0);
    models[VEHICLE_INDEX].modelTransform.translation = vec3(0.0, 1.0, 0.0);
}

void init()
{
    models[SKYBOX_INDEX].set_skybox(true);
    
    // setup vaos
    for(int i = 0; i < NUMBER_MODELS; i++){
        models[i].setup_vao();
    }
    
    for(int i = 0; i < SH_NUM_PROGRAM_TYPES; i++){
        programs[i] = *new ShaderProgram(static_cast<SH_PROGRAM_TYPE>(i));
    }
    
    for(int i = 0; i < NUMBER_MODELS; i++){
        models[i].load_model(model_filenames[i]);
    }
    
    initialise_transforms();
}

// Placeholder code for the keypress
void keypressDown(unsigned char key, int x, int y) {
    keystates[key] = true;
}

void keypressUp(unsigned char key, int x, int y){
    keystates[key] = false;
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
    
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(keypressDown);
    glutKeyboardUpFunc(keypressUp);
    
    // Set up your objects and shaders
    init();
    // Begin infinite event loop
    glutMainLoop();
    return 0;
}











