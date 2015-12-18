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

#define WORLD_LIGHT_HEIGHT 300.0

using namespace std;

int viewport_width = 800;
int viewport_height = 800;

enum CAMERA_MODE {FREE_CAMERA, THIRD_PERSON};
enum GAME_STATE {INIT_GAME, PLAY_GAME, WON_GAME, LOST_GAME, NOT_PLAYING};

#define GAME_COUNTDOWN_DURATION 30 // seconds
#define DISTANCE_COLLISION 7.0
vector<vector <float>> object_starting_points(4);
double game_countdown = 30; // time in seconds
GAME_STATE game_state = NOT_PLAYING;
int game_countdown_text_id;
float distance_from_object;
int game_distance_text;
int game_end_message_text;

CAMERA_MODE camera_mode = THIRD_PERSON;
CameraTransform camera_transform;
ThirdPersonCamera third_person_camera;
ModelTransform vehicle_transform;

bool keystates[256];

ShaderProgram programs[SH_NUM_PROGRAM_TYPES];

string *model_filenames = new string[NUMBER_MODELS]{VEHICLE_MODEL, STREET_MODEL, SKYBOX_MODEL, PICKUP_OBJECT};
vector<Model> models(NUMBER_MODELS);

#define STREET_MODEL_WIDTH 94.0 * 3
#define STREET_MODEL_LENGTH 94.0 * 3
#define STREET_MODEL_SCALE 2.0
#define STREET_GRID_SIZE 5
vector<vector <ModelTransform>> street_grid(STREET_GRID_SIZE);

float get_distance(float x1, float y1, float x2, float y2){
    float diffX = x2 - x1;
    float diffY = y2 - y1;
    return sqrtf((diffX * diffX) + (diffY * diffY));
}

void play_game(){
    static double previous_seconds = glutGet(GLUT_ELAPSED_TIME) / 1000;
    double current_seconds = glutGet(GLUT_ELAPSED_TIME) / 1000;
    double elapsed_seconds = current_seconds - previous_seconds;
    previous_seconds = current_seconds;
    string str;
    int random_object_location = rand() % 4;
    
    switch (game_state) {
        case INIT_GAME:
            distance_from_object = 0.0;
            update_text(game_end_message_text, "");
            models[VEHICLE_INDEX].modelTransform.rotation = vec3(0.0, 0.0, 0.0);
            models[VEHICLE_INDEX].modelTransform.translation = vec3(0.0, 1.0, 0.0);
            game_countdown = GAME_COUNTDOWN_DURATION;
            models[PICKUP_OBJECT_INDEX].modelTransform.scale = vec3(2.0, 2.0, 2.0);
            models[PICKUP_OBJECT_INDEX].modelTransform.translation = vec3(object_starting_points[random_object_location][0], 15.0, object_starting_points[random_object_location][1]);
            game_state = PLAY_GAME;
            break;
        case PLAY_GAME:
            game_countdown -= elapsed_seconds;
            // check if vehicle is at object
            if(game_countdown <= 0.0){
                game_state = LOST_GAME;
            }
            distance_from_object = get_distance(models[VEHICLE_INDEX].modelTransform.translation.v[0], models[VEHICLE_INDEX].modelTransform.translation.v[2], models[PICKUP_OBJECT_INDEX].modelTransform.translation.v[0], models[PICKUP_OBJECT_INDEX].modelTransform.translation.v[2]);
            if(distance_from_object <= DISTANCE_COLLISION){
                game_state = WON_GAME;
            }
            // update text on screen
            str = "Timer: " + to_string((int)game_countdown);
            update_text(game_countdown_text_id, str.c_str());
            str = "Distance: " + to_string((int)distance_from_object);
            update_text(game_distance_text, str.c_str());
            break;
        case WON_GAME:
            update_text(game_end_message_text, "WON GAME");
            break;
        case LOST_GAME:
            update_text(game_end_message_text, "LOST GAME");
            break;
        case NOT_PLAYING:
            update_text(game_end_message_text, "");
            update_text(game_distance_text, "");
            update_text(game_countdown_text_id, "");
            break;
        default:
            break;
    }
}

void display(){
    
    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for(int i = 0; i < SH_NUM_PROGRAM_TYPES; i++){
        glUseProgram(programs[i].programID);
        
        //Declarell your uniform variables that will be used in your shaders
        int view_mat_location = glGetUniformLocation (programs[i].programID, SH_UNIFORM_VIEW);
        int proj_mat_location = glGetUniformLocation (programs[i].programID, SH_UNIFORM_PERSPECTIVE);
        int wlp_location = glGetUniformLocation(programs[i].programID, SH_UNIFORM_LIGHT_POSITION);
        
        // values of matrices
        mat4 view = identity_mat4();
        vec3 world_light_position = vec3(models[VEHICLE_INDEX].modelTransform.translation.v[0], WORLD_LIGHT_HEIGHT, models[VEHICLE_INDEX].modelTransform.translation.v[2]);
        if(camera_mode == THIRD_PERSON){
            view = look_at(third_person_camera.position, third_person_camera.positionOfObject, vec3(0.0, 1.0, 0.0));
        }else if(camera_mode == FREE_CAMERA){
            view = look_at(camera_transform.eye, camera_transform.look(), vec3(0.0, 1.0, 0.0));
        }
        mat4 persp_proj = perspective(90.0, (float)viewport_width/(float)viewport_height, 0.1, 10000.0);
        
        // update uniforms & draw
        glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
        glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view.m);
        glUniform3fv(wlp_location, 1, world_light_position.v);
    }
    
    models[VEHICLE_INDEX].draw_model(programs);
    models[SKYBOX_INDEX].draw_model(programs);
    for(int i = 0; i < STREET_GRID_SIZE; i++){
        for(int j = 0; j < STREET_GRID_SIZE; j++){
            models[STREET_INDEX].modelTransform = street_grid[i][j];
            models[STREET_INDEX].draw_model(programs);
        }
    }
    if(game_state != NOT_PLAYING){
        models[PICKUP_OBJECT_INDEX].draw_model(programs);
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
    
    if(keystates['0']){
        game_state = INIT_GAME;
    }
    if(keystates['9']){
        game_state = NOT_PLAYING;
    }
    
    // object
    float inc = 4.0;
    float angle = 1.5;
    
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
    play_game();
    glutPostRedisplay();
}

void initialise_transforms(){
    camera_transform.eye = vec3(0.0, 10.0, 0.0);
    models[SKYBOX_INDEX].modelTransform.scale = vec3(1450.0, 1450.0, 1450.0);
    models[VEHICLE_INDEX].modelTransform.scale = vec3(2.0, 2.0, 2.0);
    models[VEHICLE_INDEX].modelTransform.translation = vec3(0.0, 1.0, 0.0);
}

void init(){
    models[SKYBOX_INDEX].set_skybox(true);
    
    for(int i = 0; i < STREET_GRID_SIZE; i++){
        for(int j = 0; j < STREET_GRID_SIZE; j++){
            ModelTransform tran;
            tran.scale = vec3(STREET_MODEL_SCALE, STREET_MODEL_SCALE, STREET_MODEL_SCALE);
            float offX = (i - (STREET_GRID_SIZE / 2)) * (STREET_MODEL_LENGTH * STREET_MODEL_SCALE);
            float offZ = (j - (STREET_GRID_SIZE / 2)) * (STREET_MODEL_WIDTH * STREET_MODEL_SCALE);
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
    float x = -1.0, y = 1.0, size_px = 30.0, r = 1.0, g = 0.0, b = 0.0, a = 1.0;
    game_countdown_text_id = add_text ("", x, y, size_px, r, g, b, a);
    game_distance_text = add_text("", x, y - 0.08, size_px, r, g, b, a);
    game_end_message_text = add_text("", -0.5, 0.5, size_px, r, g, b, a);
    
    object_starting_points[0].push_back(-1140.0);
    object_starting_points[0].push_back(-1165.0);
    object_starting_points[1].push_back(-1140.0);
    object_starting_points[1].push_back(1150.0);
    object_starting_points[2].push_back(1090.0);
    object_starting_points[2].push_back(1100.0);
    object_starting_points[3].push_back(1120.0);
    object_starting_points[3].push_back(-1120.0);
    
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
    glutCreateWindow("Game");
    
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
