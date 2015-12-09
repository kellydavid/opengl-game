//
//  game_object.hpp
//  opengl-game
//
//  Created by David Kelly on 09/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef game_object_hpp
#define game_object_hpp

#include <stdio.h>
#include "model.hpp"
#include "utilities.hpp"
#include "shaders.hpp"

class GameObject{
private:
    ModelData modelData;
    
    void upload_model_transform(ShaderProgram program);
public:
    ModelTransform modelTransform;
    
    GameObject();
    
    void setup_vao();
    
    void load_model(string path_to_model);
    
    void draw_model(ShaderProgram *shaderPrograms);
};

#endif /* game_object_hpp */
