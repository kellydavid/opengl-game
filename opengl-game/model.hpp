//
//  model.hpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef model_hpp
#define model_hpp

#include <iostream>
#include <stdio.h>
#include <Opengl/gl3.h>
#include <GLUT/GLUT.h>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <vector>
#include <string>

#define RELATIVE_PATH_TO_MODELS "../../opengl-game/Models/"
#define NUMBER_MODELS 2
#define VEHICLE_MODEL "vehicle.dae"
#define STREET_MODEL "street2.obj"
#define VEHICLE_INDEX 0
#define STREET_INDEX 1

using namespace std;

class ModelData;
class TextureData;
class MeshData;

class ModelData{
private:
    string file_name; // path to file where model is stored
    
    // loads in model from file
    bool load_model();
    
    bool load_texture (const char* file_name, GLuint* tex);
    
    void generate_object_buffer();
    
public:
    unsigned int vao; // points to the vao associated with the model
    // coordinates
    vector<float> g_vp, g_vn, g_vt, g_vc; // vertex positions, vertex normals, vertex textures (coordinates)
    int g_point_count = 0; // total count of points
    int g_vt_point_count = 0;
    // textures
    int num_texs = 0; // length of texture pointer array
    TextureData *textures;
    // meshes
    int num_meshes = 0; // number of meshes
    MeshData *meshes;
    
    // default constructor
    ModelData();
    
    // reads the model, binds the vao and loads the meshes into vbo
    // also loads textures
    void load_data(string filename);
    
    // initialises the vao
    void setup_vao();
    
    // get full relative path to model
    string getPath();
    
    // set the name of the file for the model
    void set_filename(string file_name);
};

class TextureData{
public:
    bool texture;
    aiColor3D color;
    string path;
    GLuint tex;

    void setPath(aiString path);
    
    string getRelPath();
};

class MeshData{
public:
    int texture_index;
    int vertex_start;
    int vertex_end;
    
    // returns number of vertices that the texture is assigned to
    int getVerts();
};

#endif /* model_hpp */
