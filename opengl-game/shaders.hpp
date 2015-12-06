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
#define COLOUR_VERTEX_SHADER "../../opengl-game/Shaders/colourVertexShader.glsl"
#define COLOUR_FRAGMENT_SHADER "../../opengl-game/Shaders/colourFragmentShader.glsl"
#define TEXTURE_VERTEX_SHADER "../../opengl-game/Shaders/textureVertexShader.glsl"
#define TEXTURE_FRAGMENT_SHADER "../../opengl-game/Shaders/textureFragmentShader.glsl"


// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile);

void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);

GLuint CreateShaderProgram();

GLuint CompileShaders(GLuint shaderProgramID);

#endif /* shaders_hpp */
