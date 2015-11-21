//
//  model.cpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "model.hpp"
#include <GLUT/GLUT.h>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// STB image loading header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

string ModelData::getPath(){
    return RELATIVE_PATH_TO_MODELS + file_name;
}

// loads in model from file
bool load_model (ModelData *model) {
    const aiScene* scene = aiImportFile (model->getPath().c_str(), aiProcess_Triangulate); // TRIANGLES!
    fprintf (stderr, "Reading mesh %s\n", model->getPath().c_str());
    if (!scene) {
        fprintf (stderr, "ERROR: reading mesh %s\n", model->getPath().c_str());
        return false;
    }
    printf ("  %i animations\n", scene->mNumAnimations);
    printf ("  %i cameras\n", scene->mNumCameras);
    printf ("  %i lights\n", scene->mNumLights);
    printf ("  %i materials\n", scene->mNumMaterials);
    printf ("  %i meshes\n", scene->mNumMeshes);
    printf ("  %i textures\n", scene->mNumTextures);
    
    model->num_meshes = scene->mNumMeshes;
    model->mesh_vertex_start_end = new int*[model->num_meshes];
    model->mesh_material_index = new int[model->num_meshes];
    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        printf ("    %i vertices in mesh\n", mesh->mNumVertices);
        model->mesh_vertex_start_end[m_i] = new int[2];
        model->mesh_vertex_start_end[m_i][0] = model->g_point_count;
        model->g_point_count += mesh->mNumVertices;
        model->mesh_vertex_start_end[m_i][1] = model->g_point_count;
        model->mesh_material_index[m_i] = mesh->mMaterialIndex;
        //cout << model->mesh_material_index[m_i] << endl;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions ()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                //printf ("      vp %i (%f,%f,%f)\n", v_i, vp->x, vp->y, vp->z);
                model->g_vp.push_back (vp->x);
                model->g_vp.push_back (vp->y);
                model->g_vp.push_back (vp->z);
            }
            if (mesh->HasNormals ()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                //printf ("      vn %i (%f,%f,%f)\n", v_i, vn->x, vn->y, vn->z);
                model->g_vn.push_back (vn->x);
                model->g_vn.push_back (vn->y);
                model->g_vn.push_back (vn->z);
            }
            if (mesh->HasTextureCoords (0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                //printf ("      vt %i (%f,%f)\n", v_i, vt->x, vt->y);
                model->g_vt.push_back (vt->x);
                model->g_vt.push_back (vt->y);
            }
            if (mesh->HasTangentsAndBitangents ()) {
                // NB: could store/print tangents here
            }
        }
    }
    
    // if scene has materials then load each texture
    if(scene->HasMaterials()){
        // bind the mesh vao
        glBindVertexArray(model->vao);
        model->num_textures = scene->mNumMaterials;
        model->tex_array = new GLuint[scene->mNumMaterials];
        for(int i = 0; i < scene->mNumMaterials; i++){
            for(int j = 0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE); j++){
                aiString path;
                scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path);
                char relpath[] = RELATIVE_PATH_TO_MODELS;
                // if path == "" then load the colour data instead TODO
                load_texture(strcat(relpath, path.C_Str()), &(model->tex_array[i]));
            }
        }
    }
    
    aiReleaseImport (scene);
    return true;
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