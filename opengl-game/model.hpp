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
#include "shaders.hpp"
#include "transform.hpp"

#define RELATIVE_PATH_TO_MODELS "../../opengl-game/Models/"
#define NUMBER_MODELS 2
#define VEHICLE_MODEL "vehicle.dae"
#define STREET_MODEL "street2.obj"
#define VEHICLE_INDEX 0
#define STREET_INDEX 1


class Model;
class ModelData;
class TextureData;
class MeshData;

class ModelData{
private:
    std::string file_name; // path to file where model is stored
    
    // loads in model from file
    bool load_model();
    
    bool load_texture (const char* file_name, GLuint* tex);
    
    void generate_object_buffer();
    
public:
    unsigned int vao; // points to the vao associated with the model
    // coordinates
    std::vector<float> g_vp, g_vn, g_vt, g_vc; // vertex positions, vertex normals, vertex textures (coordinates)
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
    void load_data(std::string filename);
    
    // initialises the vao
    void setup_vao();
    
    // get full relative path to model
    std::string getPath();
    
    // set the name of the file for the model
    void set_filename(std::string file_name);
};

class TextureData{
public:
    bool texture;
    vec3 color;
    std::string path;
    GLuint tex;

    void setPath(std::string path);
    
    std::string getRelPath();
};

class MeshData{
public:
    int texture_index;
    int vertex_start;
    int vertex_end;
    
    // returns number of vertices that the texture is assigned to
    int getVerts();
};

class Model{
private:
    ModelData modelData;
    
    void upload_model_transform(ShaderProgram program);
public:
    ModelTransform modelTransform;
    
    Model();
    
    void setup_vao();
    
    void load_model(std::string path_to_model);
    
    void draw_model(ShaderProgram *shaderPrograms);
};

#endif /* model_hpp */
