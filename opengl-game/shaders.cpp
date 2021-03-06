//
//  shaders.cpp
//  opengl-game
//
//  Created by David Kelly on 21/11/2015.
//  Copyright © 2015 David Kelly. All rights reserved.
//

#include "shaders.hpp"

char* ShaderProgram::readShaderSource(const char* shaderFile) {
    FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?
    
    if ( fp == NULL ) { return NULL; }
    
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    
    fseek(fp, 0L, SEEK_SET);
    char* buf = new char[size + 1];
    fread(buf, 1, size, fp);
    buf[size] = '\0';
    
    fclose(fp);
    
    return buf;
}

ShaderProgram::ShaderProgram(SH_PROGRAM_TYPE type): ShaderProgram(){
    this->Initialise();
    switch(type){
        case SH_COL_PROG:
            this->AddShader(SH_COLOUR_VERTEX_SHADER, GL_VERTEX_SHADER);
            this->AddShader(SH_COLOUR_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
            break;
        case SH_TEX_PROG:
            this->AddShader(SH_TEXTURE_VERTEX_SHADER, GL_VERTEX_SHADER);
            this->AddShader(SH_TEXTURE_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
            break;
        case SH_SKYBOX_PROG:
            this->AddShader(SH_SKYBOX_VERTEX_SHADER, GL_VERTEX_SHADER);
            this->AddShader(SH_SKYBOX_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
            break;
    }
    this->Compile();
}

ShaderProgram::ShaderProgram(){}

void ShaderProgram::Initialise(){
    //Start the process of setting up our shaders by creating a program ID
    //Note: we will link all the shaders together into this ID
    GLuint shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }
    this->programID = shaderProgramID;
}

void ShaderProgram::AddShader(const char* pShaderText, GLenum ShaderType){
    // create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);
    
    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
    const char* pShaderSource = readShaderSource( pShaderText);
    
    // Bind the source code to the shader, this happens before compilation
    glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
    // compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
    // check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    // Attach the compiled shader object to the program object
    glAttachShader(this->programID, ShaderObj);
}

void ShaderProgram::Compile(){
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };
    // After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(this->programID);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(this->programID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(this->programID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    
    // program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(this->programID);
    // check for program related errors using glGetProgramiv
    glGetProgramiv(this->programID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(this->programID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    // Finally, use the linked shader program
    // Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(this->programID);
}