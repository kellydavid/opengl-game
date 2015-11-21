//
//  shaders.hpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#ifndef shaders_hpp
#define shaders_hpp

#include <stdio.h>
#include <OpenGl/gl3.h>
#include <stdlib.h>

/* -----------------------
 SHADER FILEPATHS
 --------------------------*/
#define VERTEX_SHADER "../../opengl-game/Shaders/simpleVertexShader.glsl"
#define FRAGMENT_SHADER "../../opengl-game/Shaders/simpleFragmentShader.glsl"

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile);

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);

GLuint CompileShaders();

#endif /* shaders_hpp */
