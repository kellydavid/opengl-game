//
//  game_object.cpp
//  opengl-game
//
//  Created by David Kelly on 09/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "game_object.hpp"

GameObject::GameObject(){}

void GameObject::load_model(string path_to_model){
    this->modelData.load_data(path_to_model);
}

void GameObject::draw_model(ShaderProgram *shaderPrograms/*, mat4 view, mat4 persp*/){
    // setup the shaders
    for(int i = 0; i < SH_NUM_PROGRAM_TYPES; i++){
        glUseProgram(shaderPrograms[i].programID);
        glBindVertexArray(this->modelData.vao);
        upload_model_transform(shaderPrograms[i]);
    }
    
    // draw each mesh using appropriate shader
    for(int i = 0; i < this->modelData.num_meshes; i++){
        MeshData mesh = modelData.meshes[i];
        if(this->modelData.textures[mesh.texture_index].texture){
            glUseProgram(shaderPrograms[SH_TEX_PROG].programID);
            glBindTexture(GL_TEXTURE_2D, this->modelData.textures[mesh.texture_index].tex);
        }
        else{
            glUseProgram(shaderPrograms[SH_COL_PROG].programID);
            int color_location = glGetUniformLocation(shaderPrograms[SH_COL_PROG].programID, SH_UNIFORM_COLOR);
            aiColor3D color = this->modelData.textures[mesh.texture_index].color;
            float color_vec[3] = {color.r, color.g, color.b};
            glUniform3fv(color_location, 1, color_vec);
        }
        glDrawArrays(GL_TRIANGLES, mesh.vertex_start, mesh.getVerts());
    }
}

void GameObject::setup_vao(){
    modelData.setup_vao();
}

void GameObject::upload_model_transform(ShaderProgram program){
    // rotate and translate vehicle
    mat4 model = identity_mat4 ();
    rotate_mat4(&model, this->modelTransform.rotation);
    model = translate(model, this->modelTransform.translation);
    model = scale(model, this->modelTransform.scale);
    //update model matrix on gpu
    int loc_model = glGetUniformLocation(program.programID, SH_UNIFORM_MODEL);
    glUniformMatrix4fv (loc_model, 1, GL_FALSE, model.m);
}