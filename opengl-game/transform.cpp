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

void ThirdPersonCamera::calculate_position(){
    float vert = this->calculateVerticalDistance();
    float hor = this->calculateHorizontalDistance();
    this->position.v[1] = vert + this->positionOfObject.v[1];
    float horizontal_distance = hor;
    float theta = this->angleAroundObject + this->rotationOfObject.v[1];
    float offsetX = (float) horizontal_distance * sin(deg_to_radians(theta));
    float offsetZ = (float) horizontal_distance * cos(deg_to_radians(theta));
    this->position.v[0] = this->positionOfObject.v[0] + offsetX;
    this->position.v[2] = this->positionOfObject.v[2] + offsetZ;
    this->yaw = 180 - (this->rotationOfObject.v[1] + angleAroundObject);
}

float ThirdPersonCamera::calculateHorizontalDistance(){
    return (float) this->distanceFromObject * cos(deg_to_radians(pitch));
}

float ThirdPersonCamera::calculateVerticalDistance(){
    return (float) this->distanceFromObject * sin(deg_to_radians(pitch));
}

mat4 ThirdPersonCamera::get_view(){
    mat4 view = identity_mat4();
    rotate_mat4(&view, vec3(deg_to_radians(pitch), deg_to_radians(yaw), deg_to_radians(roll)));
    scale(view, this->scaleOfObject);
    vec3 neg_position = vec3(-this->position.v[0],  -this->position.v[1], -this->position.v[2]);
    view = translate(view, neg_position);
    return view;
}

float deg_to_radians(float deg){
    return (float) deg * (M_PI / 180);
}