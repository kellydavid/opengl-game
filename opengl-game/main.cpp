#include <GLUT/GLUT.h>
#include <OpenGl/gl3.h>
#include <iostream>
#include <math.h>
#include "maths_funcs.h"
#include "shaders.hpp"
#include "transform.hpp"
#include "model.hpp"
#include "text.h"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

int viewport_width = 800;
int viewport_height = 800;

enum CAMERA_MODE {FREE_CAMERA, THIRD_PERSON};


CAMERA_MODE camera_mode = THIRD_PERSON;
CameraTransform camera_transform;
ThirdPersonCamera third_person_camera;
ModelTransform vehicle_transform;

bool keystates[256];

ShaderProgram programs[SH_NUM_PROGRAM_TYPES];

string *model_filenames = new string[NUMBER_MODELS]{VEHICLE_MODEL, STREET_MODEL, SKYBOX_MODEL};
vector<Model> models(NUMBER_MODELS);

#define STREET_MODEL_WIDTH 96.717
#define STREET_MODEL_LENGTH 94.192
#define STREET_MODEL_SCALE 6.0
#define STREET_GRID_SIZE 5
vector<vector <ModelTransform>> street_grid(STREET_GRID_SIZE);

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
        
        if(camera_mode == THIRD_PERSON){
            view = look_at(third_person_camera.position, third_person_camera.positionOfObject, vec3(0.0, 1.0, 0.0));
        }else if(camera_mode == FREE_CAMERA){
            view = look_at(camera_transform.eye, camera_transform.look(), vec3(0.0, 1.0, 0.0));
        }
        mat4 persp_proj = perspective(90.0, (float)viewport_width/(float)viewport_height, 0.1, 10000.0);
        
        // update uniforms & draw
        glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
        glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view.m);
    }
    
    models[VEHICLE_INDEX].draw_model(programs);
    models[SKYBOX_INDEX].draw_model(programs);
    for(int i = 0; i < STREET_GRID_SIZE; i++){
        for(int j = 0; j < STREET_GRID_SIZE; j++){
            models[STREET_INDEX].modelTransform = street_grid[i][j];
            models[STREET_INDEX].draw_model(programs);
        }
    }
    
    draw_texts();
    
    glutSwapBuffers();
}

void UpdateKeys(){
    bool turn_right = false, turn_left = false, up = false, down = false, forward = false, back = false;
    bool p_right = false, p_left = false, p_forward = false, p_backward = false;
    float model_moved = false;
    
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
        model_moved = true;
    }
    if(keystates['k']){
        p_backward = true;
        model_moved = true;
    }
    if(keystates['j']){
        p_left = true;
        model_moved = true;
    }
    if(keystates['l']){
        p_right = true;
        model_moved = true;
    }
    
    // object
    float inc = 1.5;
    float angle = 5.0;
    
    if(p_forward){
        float radians_angle = models[VEHICLE_INDEX].modelTransform.rotation.v[1] * (M_PI / 180);
        models[VEHICLE_INDEX].modelTransform.translation.v[0] += sin(radians_angle) * inc;
        models[VEHICLE_INDEX].modelTransform.translation.v[2] += cos(radians_angle) * inc;
    }
    if(p_backward){
        float radians_angle = models[VEHICLE_INDEX].modelTransform.rotation.v[1] * (M_PI / 180);
        models[VEHICLE_INDEX].modelTransform.translation.v[0] -= sin(radians_angle) * inc;
        models[VEHICLE_INDEX].modelTransform.translation.v[2] -= cos(radians_angle) * inc;
    }
    if(p_left){
        models[VEHICLE_INDEX].modelTransform.rotation.v[1] += angle;
    }
    if(p_right){
        models[VEHICLE_INDEX].modelTransform.rotation.v[1] -= angle;
    }
    
    
    // camera
    if(keystates['1']){
        camera_mode = FREE_CAMERA;
    }
    if(keystates['2']){
        camera_mode = THIRD_PERSON;
    }
    if(camera_mode == FREE_CAMERA){
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
    }else if(camera_mode == THIRD_PERSON){
        third_person_camera.positionOfObject = models[0].modelTransform.translation;
        third_person_camera.rotationOfObject = models[0].modelTransform.rotation;
        third_person_camera.scaleOfObject = models[0].modelTransform.scale;
        third_person_camera.calculate_position();
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
    models[SKYBOX_INDEX].modelTransform.scale = vec3(1200.0, 1200.0, 1200.0);
    models[VEHICLE_INDEX].modelTransform.scale = vec3(2.0, 2.0, 2.0);
    models[VEHICLE_INDEX].modelTransform.translation = vec3(0.0, 1.0, 0.0);
}

void init()
{
    models[SKYBOX_INDEX].set_skybox(true);
    
    for(int i = 0; i < STREET_GRID_SIZE; i++){
        for(int j = 0; j < STREET_GRID_SIZE; j++){
            ModelTransform tran;
            tran.scale = vec3(STREET_MODEL_SCALE, STREET_MODEL_SCALE, STREET_MODEL_SCALE);
            float offX = (i - (STREET_GRID_SIZE / 2)) * ((STREET_MODEL_LENGTH + 40.0) / 2);
            float offZ = (j - (STREET_GRID_SIZE / 2)) * ((STREET_MODEL_WIDTH + 40.0) / 2);
            cout << "Street (" << i << ", " << j << ") x:" << to_string(offX) << " z:" << offZ << endl;
            tran.translation = vec3(offX, 0.0, offZ);
            street_grid[i].push_back(tran);
        }
    }
    
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
    
    // initialise text
    init_text_rendering ("../../opengl-game/text/freemono.png", "../../opengl-game/text/freemono.meta", viewport_width, viewport_height);
    // x and y are -1 to 1
    // size_px is the maximum glyph size in pixels (try 100.0f)
    // r,g,b,a are red,blue,green,opacity values between 0.0 and 1.0
    // if you want to change the text later you will use the returned integer as a parameter
    float x = -1.0, y = 1.0, size_px = 20.0, r = 1.0, g = 0.0, b = 0.0, a = 1.0;
    int hello_id = add_text ("Hello world!", x, y, size_px, r, g, b, a);
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
    glutInitWindowSize(viewport_width, viewport_height);
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
