//
//  utilities.hpp
//  opengl-game
//
//  Created by David Kelly on 04/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef transform_hpp
#define transform_hpp

#include <OpenGl/gl3.h>
#include <GLUT/GLUT.h>
#include <iostream>
#include <math.h>
#include "maths_funcs.h"
#include <stdio.h>

class ModelTransform{
public:
    vec3 translation = vec3(0.0, 0.0, 0.0);
    vec3 rotation = vec3(0.0, 0.0, 0.0);
    vec3 scale = vec3(1.0, 1.0, 1.0);
    
    // prints out the values of the transform
    void print_values();
};

class CameraTransform{
public:
    const float fraction = 5.5;
    const float angle_inc = 0.1;
    float angle = 0.0;
    float lx = 0.0, lz = -1.0f;
    vec3 eye;
    
    vec3 look();
};

class ThirdPersonCamera{
private:
    float calculateHorizontalDistance();
    
    float calculateVerticalDistance();
public:
    vec3 position = vec3(0.0, 0.0, 0.0);
    
    float pitch = 30.0;
    float yaw = 0.0;
    float roll = 0.0;
    
    float distanceFromObject = 80.0;
    float angleAroundObject = 0.0;
    
    vec3 positionOfObject;
    vec3 rotationOfObject;
    vec3 scaleOfObject;
    
    void calculate_position();
    
    mat4 get_view();
};

void rotate_mat4(mat4 *mat, vec3 rotation);

float deg_to_radians(float deg);

#endif /* transform_hpp */
