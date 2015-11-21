//
//  shaders.hpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef shaders_hpp
#define shaders_hpp

/* -----------------------
 SHADER FILEPATHS
 --------------------------*/
#define VERTEX_SHADER "../../opengl-game/Shaders/simpleVertexShader.glsl"
#define FRAGMENT_SHADER "../../opengl-game/Shaders/simpleFragmentShader.glsl"

#include <stdio.h>
#include <OpenGl/gl3.h>
#include <stdlib.h>


// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile);

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);

GLuint CompileShaders();

#endif /* shaders_hpp */
