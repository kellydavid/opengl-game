//
//  shaders.hpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef shaders_hpp
#define shaders_hpp

#include <iostream>
#include <stdio.h>
#include <OpenGl/gl3.h>
#include <stdlib.h>
#include <vector>

/* -----------------------
 SHADER FILEPATHS
 --------------------------*/
#define SH_COLOUR_VERTEX_SHADER "../../opengl-game/Shaders/colourVertexShader.glsl"
#define SH_COLOUR_FRAGMENT_SHADER "../../opengl-game/Shaders/colourFragmentShader.glsl"
#define SH_TEXTURE_VERTEX_SHADER "../../opengl-game/Shaders/textureVertexShader.glsl"
#define SH_TEXTURE_FRAGMENT_SHADER "../../opengl-game/Shaders/textureFragmentShader.glsl"
#define SH_SKYBOX_VERTEX_SHADER "../../opengl-game/Shaders/skyboxVertexShader.glsl"
#define SH_SKYBOX_FRAGMENT_SHADER "../../opengl-game/Shaders/skyboxFragmentShader.glsl"

#define SH_LOCATION_VERTEX_POSITION 0
#define SH_LOCATION_VERTEX_NORMAL 1
#define SH_LOCATION_VERTEX_TEXTURE 2

#define SH_UNIFORM_MODEL "model"
#define SH_UNIFORM_VIEW "view"
#define SH_UNIFORM_PERSPECTIVE "proj"
#define SH_UNIFORM_COLOR "color"
#define SH_UNIFORM_LIGHT_POSITION "world_light_position"

#define SH_NUM_PROGRAM_TYPES 3
enum SH_PROGRAM_TYPE{SH_COL_PROG, SH_TEX_PROG, SH_SKYBOX_PROG};

class ShaderProgram{
private:
    // Create a NULL-terminated string by reading the provided file
    char* readShaderSource(const char* shaderFile);
public:
    SH_PROGRAM_TYPE type;
    GLuint programID;
    
    ShaderProgram();
    ShaderProgram(SH_PROGRAM_TYPE type);
    void Initialise();
    void AddShader(const char* pShaderText, GLenum ShaderType);
    void Compile();
};

#endif /* shaders_hpp */
