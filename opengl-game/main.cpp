#include <GLUT/GLUT.h>
#include <OpenGl/gl3.h>
#include <iostream>
#include <math.h>
#include "maths_funcs.h"
#include "shaders.hpp"
#include "model.hpp"
#include "utilities.hpp"

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

int number_of_models = NUMBER_MODELS;
string *model_filenames = new string[number_of_models]{VEHICLE_MODEL, STREET_MODEL};
ModelData *models = new ModelData[number_of_models];

int width = 800;
int height = 800;

GLuint loc1, loc2, loc3;

CameraTransform camera_transform;
ModelTransform vehicle_transform;

GLuint colourProgramID;
GLuint textureProgramID;

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
    
    //textureProgram
    loc1 = glGetAttribLocation(colourProgramID, "vertex_position");
    loc2 = glGetAttribLocation(colourProgramID, "vertex_normal");
    loc3 = glGetAttribLocation(colourProgramID, "vertex_texture");
    
    // load in vertex positions
    unsigned int vp_vbo = 0;
    glGenBuffers (1, &vp_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
    glBufferData (GL_ARRAY_BUFFER, model->g_point_count * 3 * sizeof (float), &(model->g_vp[0]), GL_STATIC_DRAW);
    
    // load in vertex normals
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

void display(){
    
    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
    glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (colourProgramID);
    
    
    //Declare your uniform variables that will be used in your shader
    int matrix_location = glGetUniformLocation (colourProgramID, "model");
    int view_mat_location = glGetUniformLocation (colourProgramID, "view");
    int proj_mat_location = glGetUniformLocation (colourProgramID, "proj");
    
    
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
        //cout << "Mesh #" << i << " material index " << models[1].meshes[i].texture_index << endl;
        if(models[1].textures[models[1].meshes[i].texture_index].texture){
            glUseProgram (colourProgramID);
            glBindTexture(GL_TEXTURE_2D, models[1].textures[models[1].meshes[i].texture_index].tex);
        }else{
            glUseProgram(colourProgramID);
            glBindVertexArray(models[1].vao);
            
            int color_location = glGetUniformLocation (colourProgramID, "color");
            aiColor3D color = models[1].textures[models[1].meshes[i].texture_index].color;
            float color_vec[3] = {color.r, color.g, color.b};
            
            matrix_location = glGetUniformLocation (colourProgramID, "model");
            view_mat_location = glGetUniformLocation (colourProgramID, "view");
            proj_mat_location = glGetUniformLocation (colourProgramID, "proj");
            
            glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
            glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view.m);
            glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model.m);
            
            glUniform3fv(color_location, 1, color_vec);
            //cout << "Mesh #" << i << " Using color R:" << color.r << " G:" << color.g << " B:" << color.b << endl;
        }
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
    // Program with the texture version of fragment shader
    textureProgramID = CreateShaderProgram();
    AddShader(textureProgramID, TEXTURE_VERTEX_SHADER, GL_VERTEX_SHADER);
    AddShader(textureProgramID, TEXTURE_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
    textureProgramID = CompileShaders(textureProgramID);
    // program with standard colour fragment shader
    colourProgramID = CreateShaderProgram();
    AddShader(colourProgramID, COLOUR_VERTEX_SHADER, GL_VERTEX_SHADER);
    AddShader(colourProgramID, COLOUR_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
    colourProgramID = CompileShaders(colourProgramID);
    // load mesh into a vertex buffer array
    loadModels();
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











