//
//  model.hpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef model_hpp
#define model_hpp

#include <stdio.h>
#include <Opengl/gl3.h>
#include <vector>
#include <string>

#define RELATIVE_PATH_TO_MODELS "../../opengl-game/Models/"
#define NUMBER_MODELS 2
#define VEHICLE_MODEL "vehicle.dae"
#define STREET_MODEL "street2.obj"
#define VEHICLE_INDEX 0
#define STREET_INDEX 1

using namespace std;

class ModelData{
public:
    string file_name; // path to file where model is stored
    unsigned int vao; // points to the vao associated with the model
    vector<float> g_vp, g_vn, g_vt; // vertex positions, vertex normals, vertex textures (coordinates)
    int g_point_count = 0; // total count of points
    int num_textures = 0; // length of texture pointer array
    GLuint *tex_array; // array of tex pointers
    int num_meshes = 0; // number of meshes
    int **mesh_vertex_start_end; // array storing start and end indices of vertices
    int *mesh_material_index; // the index refers to the mesh and the value refers to the index of the material
    
    // get full relative path to model
    string getPath();
};


bool load_model (ModelData *model);

bool load_texture (const char* file_name, GLuint* tex);


#endif /* model_hpp */
