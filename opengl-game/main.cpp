#include <GLUT/GLUT.h>
#include <OpenGl/gl3.h>
#include <iostream>
#include <math.h>
#include "maths_funcs.h"
#include "shaders.hpp"
#include "model.hpp"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

int number_of_models = NUMBER_MODELS;
string *model_filenames = new string[number_of_models]{VEHICLE_MODEL, STREET_MODEL};
ModelData *models = new ModelData[number_of_models];

int width = 800;
int height = 800;

GLuint loc1, loc2, loc3;

class ModelTransform{
public:
    vec3 translation = vec3(0.0, 0.0, 0.0);
    vec3 rotation = vec3(0.0, 0.0, 0.0);
    vec3 scale = vec3(0.0, 0.0, 0.0);
    
    void print_values(){
        cout << "Translation: (" << translation.v[0] << ", " << translation.v[1] << ", " << translation.v[2] << ")" << endl;
        cout << "Rotation: (" << rotation.v[0] << ", " << rotation.v[1] << ", " << rotation.v[2] << ")" << endl;
        cout << "Scale: (" << scale.v[0] << ", " << scale.v[1] << ", " << scale.v[2] << ")" << endl;
    }
};

class CameraTransform{
public:
    const float fraction = 2.5;
    const float angle_inc = 0.1;
    float angle = 0.0;
    float lx = 0.0, lz = -1.0f;
    vec3 eye;
    
    vec3 look(){
        return vec3(eye.v[0] + lx, eye.v[1], eye.v[2] + lz);
    }
};

CameraTransform camera_transform;
ModelTransform vehicle_transform;

GLuint shaderProgramID;

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferMesh(ModelData *model) {
    /*----------------------------------------------------------------------------
     LOAD model HERE AND COPY INTO BUFFERS
     ----------------------------------------------------------------------------*/
    
    //Note: you may get an error "vector subscript out of range" if you are using this code for a model that doesnt have positions and normals
    //Might be an idea to do a check for that before generating and binding the buffer.
    
    // bind the vertex array object
    glBindVertexArray(model->vao);
    
    load_model (model);
    unsigned int vp_vbo = 0;
    loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
    loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
    loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
    
    glGenBuffers (1, &vp_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
    glBufferData (GL_ARRAY_BUFFER, model->g_point_count * 3 * sizeof (float), &(model->g_vp[0]), GL_STATIC_DRAW);
    unsigned int vn_vbo = 0;
    glGenBuffers (1, &vn_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
    glBufferData (GL_ARRAY_BUFFER, model->g_point_count * 3 * sizeof (float), &(model->g_vn[0]), GL_STATIC_DRAW);
    
    //	This is for texture coordinates which you don't currently need, so I have commented it out
    unsigned int vt_vbo = 0;
    glGenBuffers (1, &vt_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    glBufferData (GL_ARRAY_BUFFER, model->g_point_count * 2 * sizeof (float), &model->g_vt[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray (loc1);
    glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer (loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray (loc2);
    glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer (loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    //	This is for texture coordinates which you don't currently need, so I have commented it out
    glEnableVertexAttribArray (loc3);
    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
}


#pragma endregion VBO_FUNCTIONS

void rotate_mat4(mat4 *mat, vec3 rotation){
    *mat = rotate_x_deg(*mat, rotation.v[0]);
    *mat = rotate_y_deg(*mat, rotation.v[1]);
    *mat = rotate_z_deg(*mat, rotation.v[2]);
}

void display(){
    
    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (shaderProgramID);
    
    
    //Declare your uniform variables that will be used in your shader
    int matrix_location = glGetUniformLocation (shaderProgramID, "model");
    int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");
    
    
    // Root of the Hierarchy
    mat4 view = identity_mat4 ();
    view = look_at(camera_transform.eye, camera_transform.look(), vec3(0.0, 1.0, 0.0));
    mat4 persp_proj = perspective(90.0, (float)width/(float)height, 0.1, 10000.0);
    mat4 model = identity_mat4 ();
    model = scale(model, vec3(6.0, 6.0, 6.0));
    
    // update uniforms & draw
    glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
    glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view.m);
    glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model.m);
    
    // draw street
    glBindVertexArray(models[1].vao);
    for(int i = 0; i < models[1].num_meshes; i++){
        //cout << "Mesh #" << i<< " material index " << models[1].mesh_material_index[i] << endl;
        glBindTexture(GL_TEXTURE_2D, models[1].textures[models[1].meshes[i].texture_index].tex);
        glDrawArrays(GL_TRIANGLES, models[1].meshes[i].vertex_start, models[1].meshes[i].getVerts());
    }
    
    // rotate and translate vehicle
    model = identity_mat4 ();
    rotate_mat4(&model, vehicle_transform.rotation);
    model = translate(model, vehicle_transform.translation);
    model = scale(model, vehicle_transform.scale);
    
    //update model matrix on gpu
    glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model.m);
    
    // draw vehicle
    glBindVertexArray(models[0].vao);
    glDrawArrays (GL_TRIANGLES, 0, models[0].g_point_count);
    
    
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

void setupVAOs(){
    for(int i = 0; i < number_of_models; i++){
        // create vao for model
        models[i].vao = 0;
        glGenVertexArrays(1, &models[i].vao);
        glBindVertexArray(models[i].vao);
    }
}

void loadModels(){
    for(int i = 0; i < number_of_models; i++){
        models[i].file_name = model_filenames[i];
        generateObjectBufferMesh(&models[i]);
        // load textures
    }
}

void initialise_transforms(){
    camera_transform.eye = vec3(0.0, 10.0, 0.0);
    vehicle_transform.scale = vec3(0.8, 0.8, 0.8);
    vehicle_transform.rotation = vec3(-90.0, 0.0, 0.0);
    vehicle_transform.translation = vec3(-90.0, 0.0, 180.0);
}

void init()
{
    // set up vaos
    setupVAOs();
    // intialise transformations
    initialise_transforms();
    // Set up the shaders
    shaderProgramID = CompileShaders();
    // load mesh into a vertex buffer array
    loadModels();
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    if(key == 'a'){
        camera_transform.angle -= camera_transform.angle_inc;
        camera_transform.lx = sin(camera_transform.angle);
        camera_transform.lz = -cos(camera_transform.angle);
    }
    if(key == 'd'){
        camera_transform.angle += camera_transform.angle_inc;
        camera_transform.lx = sin(camera_transform.angle);
        camera_transform.lz = -cos(camera_transform.angle);
    }
    if(key == 'w'){
        camera_transform.eye.v[0] += camera_transform.lx * camera_transform.fraction;
        camera_transform.eye.v[2] += camera_transform.lz * camera_transform.fraction;
    }
    if(key == 's'){
        camera_transform.eye.v[0] -= camera_transform.lx * camera_transform.fraction;
        camera_transform.eye.v[2] -= camera_transform.lz * camera_transform.fraction;
    }
    if(key == 'e'){
        camera_transform.eye.v[1] += camera_transform.fraction;
    }
    if(key == 'q'){
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











