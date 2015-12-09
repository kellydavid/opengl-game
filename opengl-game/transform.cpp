//
//  utilities.cpp
//  opengl-game
//
//  Created by David Kelly on 04/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "transform.hpp"

void ModelTransform::print_values(){
    std::cout << "Translation: (" << translation.v[0] << ", " << translation.v[1] << ", " << translation.v[2] << ")" << std::endl;
    std::cout << "Rotation: (" << rotation.v[0] << ", " << rotation.v[1] << ", " << rotation.v[2] << ")" << std::endl;
    std::cout << "Scale: (" << scale.v[0] << ", " << scale.v[1] << ", " << scale.v[2] << ")" << std::endl;
}

vec3 CameraTransform::look(){
    return vec3(eye.v[0] + lx, eye.v[1], eye.v[2] + lz);
}

void rotate_mat4(mat4 *mat, vec3 rotation){
    *mat = rotate_x_deg(*mat, rotation.v[0]);
    *mat = rotate_y_deg(*mat, rotation.v[1]);
    *mat = rotate_z_deg(*mat, rotation.v[2]);
}