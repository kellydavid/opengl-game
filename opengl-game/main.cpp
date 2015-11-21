#include <GLUT/GLUT.h>
#include <OpenGl/gl3.h>
#include <iostream>
#include "maths_funcs.h"
#include "shaders.hpp"

// Assimp includes

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// STB image loading header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*----------------------------------------------------------------------------
 MESH TO LOAD
 ----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define RELATIVE_PATH_TO_MODELS "../../opengl-game/Models/"
#define VEHICLE_MESH "../../opengl-game/Models/vehicle.dae"
#define STREET_MESH "../../opengl-game/Models/street2.obj"
#define VEHICLE_MODEL 0
#define STREET_MODEL 1
/*----------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Class to represent the data associated with each mesh
class MeshData{
public:
    string file_name;
    unsigned int vao;
    std::vector<float> g_vp, g_vn, g_vt;
    int g_point_count = 0;
    int num_textures = 0;
    GLuint *tex_array; // array of tex pointers
    int num_meshes = 0;
    int **mesh_vertex_start_end; // array storing start and end indices of vertices
    int *mesh_material_index; // the index refers to the mesh and the value refers to the index of the material
};
int number_of_models = 2;
string *mesh_filenames = new string[number_of_models]{VEHICLE_MESH, STREET_MESH};
MeshData *meshes = new MeshData[number_of_models];

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

bool load_texture (const char* file_name, GLuint* tex);

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
 MESH LOADING FUNCTION
 ----------------------------------------------------------------------------*/

bool load_mesh (MeshData *meshData) {
    const aiScene* scene = aiImportFile (meshData->file_name.c_str(), aiProcess_Triangulate); // TRIANGLES!
    fprintf (stderr, "Reading mesh %s\n", meshData->file_name.c_str());
    if (!scene) {
        fprintf (stderr, "ERROR: reading mesh %s\n", meshData->file_name.c_str());
        return false;
    }
    printf ("  %i animations\n", scene->mNumAnimations);
    printf ("  %i cameras\n", scene->mNumCameras);
    printf ("  %i lights\n", scene->mNumLights);
    printf ("  %i materials\n", scene->mNumMaterials);
    printf ("  %i meshes\n", scene->mNumMeshes);
    printf ("  %i textures\n", scene->mNumTextures);
    
    meshData->num_meshes = scene->mNumMeshes;
    meshData->mesh_vertex_start_end = new int*[meshData->num_meshes];
    meshData->mesh_material_index = new int[meshData->num_meshes];
    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        printf ("    %i vertices in mesh\n", mesh->mNumVertices);
        meshData->mesh_vertex_start_end[m_i] = new int[2];
        meshData->mesh_vertex_start_end[m_i][0] = meshData->g_point_count;
        meshData->g_point_count += mesh->mNumVertices;
        meshData->mesh_vertex_start_end[m_i][1] = meshData->g_point_count;
        meshData->mesh_material_index[m_i] = mesh->mMaterialIndex;
        //cout << meshData->mesh_material_index[m_i] << endl;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions ()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                //printf ("      vp %i (%f,%f,%f)\n", v_i, vp->x, vp->y, vp->z);
                meshData->g_vp.push_back (vp->x);
                meshData->g_vp.push_back (vp->y);
                meshData->g_vp.push_back (vp->z);
            }
            if (mesh->HasNormals ()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                //printf ("      vn %i (%f,%f,%f)\n", v_i, vn->x, vn->y, vn->z);
                meshData->g_vn.push_back (vn->x);
                meshData->g_vn.push_back (vn->y);
                meshData->g_vn.push_back (vn->z);
            }
            if (mesh->HasTextureCoords (0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                //printf ("      vt %i (%f,%f)\n", v_i, vt->x, vt->y);
                meshData->g_vt.push_back (vt->x);
                meshData->g_vt.push_back (vt->y);
            }
            if (mesh->HasTangentsAndBitangents ()) {
                // NB: could store/print tangents here
            }
        }
    }
    
    // if scene has materials then load each texture
    if(scene->HasMaterials()){
        // bind the mesh vao
        glBindVertexArray(meshData->vao);
        meshData->num_textures = scene->mNumMaterials;
        meshData->tex_array = new GLuint[scene->mNumMaterials];
        for(int i = 0; i < scene->mNumMaterials; i++){
            for(int j = 0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE); j++){
                aiString path;
                scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path);
                char relpath[] = RELATIVE_PATH_TO_MODELS;
                // if path == "" then load the colour data instead TODO
                load_texture(strcat(relpath, path.C_Str()), &(meshData->tex_array[i]));
            }
        }
    }
    
    aiReleaseImport (scene);
    return true;
}

#pragma endregion MESH LOADING

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferMesh(MeshData *mesh) {
    /*----------------------------------------------------------------------------
     LOAD MESH HERE AND COPY INTO BUFFERS
     ----------------------------------------------------------------------------*/
    
    //Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
    //Might be an idea to do a check for that before generating and binding the buffer.
    
    // bind the vertex array object
    glBindVertexArray(mesh->vao);
    
    load_mesh (mesh);
    unsigned int vp_vbo = 0;
    loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
    loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
    loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
    
    glGenBuffers (1, &vp_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
    glBufferData (GL_ARRAY_BUFFER, mesh->g_point_count * 3 * sizeof (float), &(mesh->g_vp[0]), GL_STATIC_DRAW);
    unsigned int vn_vbo = 0;
    glGenBuffers (1, &vn_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
    glBufferData (GL_ARRAY_BUFFER, mesh->g_point_count * 3 * sizeof (float), &(mesh->g_vn[0]), GL_STATIC_DRAW);
    
    //	This is for texture coordinates which you don't currently need, so I have commented it out
    unsigned int vt_vbo = 0;
    glGenBuffers (1, &vt_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    glBufferData (GL_ARRAY_BUFFER, mesh->g_point_count * 2 * sizeof (float), &mesh->g_vt[0], GL_STATIC_DRAW);
    
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

bool load_texture (const char* file_name, GLuint* tex) {
    int x, y, n;
    int force_channels = 4;
    unsigned char* image_data = stbi_load (file_name, &x, &y, &n, force_channels);
    if (!image_data) {
        fprintf (stderr, "ERROR: could not load %s\n", file_name);
        return false;
    }
    // NPOT check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
        fprintf (
                 stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
                 );
    }
    int width_in_bytes = x * 4;
    unsigned char *top = NULL;
    unsigned char *bottom = NULL;
    unsigned char temp = 0;
    int half_height = y / 2;
    
    for (int row = 0; row < half_height; row++) {
        top = image_data + row * width_in_bytes;
        bottom = image_data + (y - row - 1) * width_in_bytes;
        for (int col = 0; col < width_in_bytes; col++) {
            temp = *top;
            *top = *bottom;
            *bottom = temp;
            top++;
            bottom++;
        }
    }
    glGenTextures (1, tex);
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, *tex);
    glTexImage2D (
                  GL_TEXTURE_2D,
                  0,
                  GL_RGBA,
                  x,
                  y,
                  0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  image_data
                  );
    glGenerateMipmap (GL_TEXTURE_2D);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    GLfloat max_aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    // set the maximum!
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    return true;
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
    glBindVertexArray(meshes[1].vao);
    for(int i = 0; i < meshes[1].num_meshes; i++){
        //cout << "Mesh #" << i<< " material index " << meshes[1].mesh_material_index[i] << endl;
        glBindTexture(GL_TEXTURE_2D, meshes[1].tex_array[meshes[1].mesh_material_index[i]]);
        glDrawArrays(GL_TRIANGLES, meshes[1].mesh_vertex_start_end[i][0], meshes[1].mesh_vertex_start_end[i][1] - meshes[1].mesh_vertex_start_end[i][0]);
    }
    glBindTexture(GL_TEXTURE_2D, meshes[1].tex_array[2]);
    glDrawArrays (GL_TRIANGLES, 0, meshes[1].g_point_count);
    
    // rotate and translate vehicle
    model = identity_mat4 ();
    rotate_mat4(&model, vehicle_transform.rotation);
    model = translate(model, vehicle_transform.translation);
    model = scale(model, vehicle_transform.scale);
    
    //update model matrix on gpu
    glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model.m);
    
    // draw vehicle
    glBindVertexArray(meshes[0].vao);
    glDrawArrays (GL_TRIANGLES, 0, meshes[0].g_point_count);
    
    
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
        meshes[i].vao = 0;
        glGenVertexArrays(1, &meshes[i].vao);
        glBindVertexArray(meshes[i].vao);
    }
}

void loadModels(){
    for(int i = 0; i < number_of_models; i++){
        meshes[i].file_name = mesh_filenames[i];
        generateObjectBufferMesh(&meshes[i]);
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











