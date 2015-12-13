//
//  model.cpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "model.hpp"
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <vector>
#include <string>

// STB image loading header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

string skybox_images[NUMBER_SKYBOX_IMAGES] = {
    SKYBOX_GRAY_SKY_POSX, SKYBOX_GRAY_SKY_NEGX,
    SKYBOX_GRAY_SKY_POSY, SKYBOX_GRAY_SKY_NEGY,
    SKYBOX_GRAY_SKY_POSZ, SKYBOX_GRAY_SKY_NEGZ
};

/**** Model ****/
Model::Model(){}

void Model::load_model(string path_to_model){
    this->modelData.load_data(path_to_model);
}

void Model::draw_model(ShaderProgram *shaderPrograms/*, mat4 view, mat4 persp*/){
    // setup the shaders
    for(int i = 0; i < SH_NUM_PROGRAM_TYPES; i++){
        glUseProgram(shaderPrograms[i].programID);
        glBindVertexArray(this->modelData.vao);
        upload_model_transform(shaderPrograms[i]);
    }
    
    // draw each mesh using appropriate shader
    for(int i = 0; i < this->modelData.num_meshes; i++){
        MeshData mesh = modelData.meshes[i];
        if(this->is_skybox()){
            glUseProgram(shaderPrograms[SH_SKYBOX_PROG].programID);
            glDepthMask(false);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->modelData.textures[mesh.texture_index].tex);
            glDepthMask(true);
        }
        else if(this->modelData.textures[mesh.texture_index].texture){
            glUseProgram(shaderPrograms[SH_TEX_PROG].programID);
            glBindTexture(GL_TEXTURE_2D, this->modelData.textures[mesh.texture_index].tex);
        }
        else{
            glUseProgram(shaderPrograms[SH_COL_PROG].programID);
            int color_location = glGetUniformLocation(shaderPrograms[SH_COL_PROG].programID, SH_UNIFORM_COLOR);
            vec3 color = this->modelData.textures[mesh.texture_index].color;
            float color_vec[3] = {color.v[0], color.v[1], color.v[2]};
            glUniform3fv(color_location, 1, color_vec);
        }
        glDrawArrays(GL_TRIANGLES, mesh.vertex_start, mesh.getVerts());
    }
}

void Model::setup_vao(){
    modelData.setup_vao();
}

void Model::upload_model_transform(ShaderProgram program){
    // rotate and translate vehicle
    mat4 model = identity_mat4 ();
    rotate_mat4(&model, this->modelTransform.rotation);
    model = translate(model, this->modelTransform.translation);
    model = scale(model, this->modelTransform.scale);
    //update model matrix on gpu
    int loc_model = glGetUniformLocation(program.programID, SH_UNIFORM_MODEL);
    glUniformMatrix4fv (loc_model, 1, GL_FALSE, model.m);
}

void Model::set_skybox(bool skybox){
    this->modelData.skybox = skybox;
}

bool Model::is_skybox(){
    return this->modelData.skybox;
}

/**** ModelData ****/

ModelData::ModelData(){
}

void ModelData::load_data(string filename){
    glBindVertexArray(this->vao);
    set_filename(filename);
    load_model();
    generate_object_buffer();
}

string ModelData::getPath(){
    return RELATIVE_PATH_TO_MODELS + this->file_name;
}

void ModelData::set_filename(string file_name){
    this->file_name = file_name;
}

void ModelData::setup_vao(){
    this->vao = 0;
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);
}

void ModelData::generate_object_buffer(){
    GLuint loc1 = SH_LOCATION_VERTEX_POSITION;
    GLuint loc2 = SH_LOCATION_VERTEX_NORMAL;
    GLuint loc3 = SH_LOCATION_VERTEX_TEXTURE;
    
    // load in vertex positions
    unsigned int vp_vbo = 0;
    glGenBuffers (1, &vp_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vp_vbo);
    glBufferData (GL_ARRAY_BUFFER, this->g_point_count * 3 * sizeof (float), &(this->g_vp[0]), GL_STATIC_DRAW);
    
    // load in vertex normals
    unsigned int vn_vbo = 0;
    glGenBuffers (1, &vn_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vn_vbo);
    glBufferData (GL_ARRAY_BUFFER, this->g_point_count * 3 * sizeof (float), &(this->g_vn[0]), GL_STATIC_DRAW);
    
    //	This is for texture coordinates which you don't currently need, so I have commented it out
    unsigned int vt_vbo = 0;
    glGenBuffers (1, &vt_vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
    glBufferData (GL_ARRAY_BUFFER, this->g_point_count * 2 * sizeof (float), &this->g_vt[0], GL_STATIC_DRAW);
    
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

bool ModelData::load_model () {
    const aiScene* scene = aiImportFile (this->getPath().c_str(), aiProcess_Triangulate); // TRIANGLES!
    fprintf (stderr, "Reading mesh %s\n", this->getPath().c_str());
    if (!scene) {
        fprintf (stderr, "ERROR: reading mesh %s\n", this->getPath().c_str());
        return false;
    }
    printf ("  %i animations\n", scene->mNumAnimations);
    printf ("  %i cameras\n", scene->mNumCameras);
    printf ("  %i lights\n", scene->mNumLights);
    printf ("  %i materials\n", scene->mNumMaterials);
    printf ("  %i meshes\n", scene->mNumMeshes);
    printf ("  %i textures\n", scene->mNumTextures);
    
    this->num_meshes = scene->mNumMeshes;
    this->meshes = new MeshData[this->num_meshes];
    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        //printf ("    %i vertices in mesh\n", mesh->mNumVertices);
        this->meshes[m_i].vertex_start = this->g_point_count;
        this->g_point_count += mesh->mNumVertices;
        this->meshes[m_i].vertex_end = this->g_point_count;
        this->meshes[m_i].texture_index = mesh->mMaterialIndex;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions ()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                //printf ("      vp %i (%f,%f,%f)\n", v_i, vp->x, vp->y, vp->z);
                this->g_vp.push_back (vp->x);
                this->g_vp.push_back (vp->y);
                this->g_vp.push_back (vp->z);
            }
            if (mesh->HasNormals ()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                //printf ("      vn %i (%f,%f,%f)\n", v_i, vn->x, vn->y, vn->z);
                this->g_vn.push_back (vn->x);
                this->g_vn.push_back (vn->y);
                this->g_vn.push_back (vn->z);
            }
            if (mesh->HasTextureCoords (0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                //printf ("      vt %i (%f,%f)\n", v_i, vt->x, vt->y);
                this->g_vt.push_back (vt->x);
                this->g_vt.push_back (vt->y);
            }
            if (mesh->HasTangentsAndBitangents ()) {
                // NB: could store/print tangents here
            }
        }
    }
    
    if(!this->skybox){
        // if scene has materials then load each texture/color
        if(scene->HasMaterials()){
            this->num_texs = scene->mNumMaterials;
            this->textures = new TextureData[this->num_texs];
            for(int i = 0; i < scene->mNumMaterials; i++){
                if(scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) != 0){
                    aiString path;
                    scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
                    this->textures[i].setPath(path.C_Str());
                    string str = this->textures[i].path;
                    load_texture(this->textures[i].getRelPath().c_str(), &this->textures[i].tex);
                    this->textures[i].texture = true;
                    //cout << "path to texture: " << str << endl;
                }else{
                    this->textures[i].texture = false;
                    aiColor3D color;
                    scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
                    this->textures[i].color.v[0] = color.r;
                    this->textures[i].color.v[1] = color.g;
                    this->textures[i].color.v[2] = color.b;
                    cout << "Texture #" << i << " R:" << this->textures[i].color.v[0] << " G:" << this->textures[i].color.v[1] << " B:" << this->textures[i].color.v[2] << endl;
                }
            }
        }
    }else{
        for(int i = 0; i < this->num_meshes; i++)
            this->meshes[i].texture_index = 0;
        this->num_texs = 1;
        this->textures = new TextureData[this->num_texs];
        this->textures[0].texture = true;
        
        glActiveTexture(GL_TEXTURE0);
        
        glGenTextures(1, &this->textures[0].tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, this->textures[0].tex);
        
        GLuint targets[] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };
        
        // load all the images
        for(int i = 0; i < NUMBER_SKYBOX_IMAGES; i++){
            load_skybox_texture(skybox_images[i].c_str(), targets[i]);
        }
        
        // cube map settings
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    
    aiReleaseImport (scene);
    return true;
}

bool ModelData::load_skybox_texture(const char * file_path, GLuint target){
    int x, y, n;
    int force_channels = 4;
    unsigned char* image_data = stbi_load (file_path, &x, &y, &n, force_channels);
    if (!image_data) {
        fprintf (stderr, "ERROR: could not load %s\n", file_path);
        return false;
    }
    // NPOT check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
        fprintf (
                 stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_path
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
    // copy image data into 'target' side of cube map
    glTexImage2D (
                  target,
                  0,
                  GL_RGBA,
                  x,
                  y,
                  0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  image_data
                  );
    free (image_data);
    return true;
    return true;
}

bool ModelData::load_texture (const char* file_name, GLuint* tex) {
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

/**** MeshData ****/

int MeshData::getVerts(){
    return vertex_end - vertex_start;
}

/**** TextureData ****/

void TextureData::setPath(string path){
    this->path = path;
}

string TextureData::getRelPath(){
    return RELATIVE_PATH_TO_MODELS + path;
}

#undef STB_IMAGE_IMPLEMENTATION