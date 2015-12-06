//
//  utilities.hpp
//  opengl-game
//
//  Created by David Kelly on 04/12/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef utilities_hpp
#define utilities_hpp

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
    vec3 scale = vec3(0.0, 0.0, 0.0);
    
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

void rotate_mat4(mat4 *mat, vec3 rotation);

#endif /* utilities_hpp */
