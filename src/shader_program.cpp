#include <fstream>
#include <sstream>
#include <iostream>
#include "shader_program.h"

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(ID);
}

void ShaderProgram::use()
{
    glUseProgram(ID);
}

void ShaderProgram::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void ShaderProgram::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void ShaderProgram::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}
void ShaderProgram::setMat4(const std::string &name, bool transpose, const GLfloat* value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, transpose, value);
}

// UTILITIES
unsigned ShaderProgram::readAndCompileShaderFile(const char* shaderPath, unsigned& shaderID)
{
    std::string shaderCode;
    std::ifstream shaderFile;
    // ensure ifstream object can throw exceptions:
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // open file
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf(); // read file's buffer contents into stream
        shaderFile.close();    // close file handler

        shaderCode = shaderStream.str();
    }
    catch(const std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* shaderCodeCStr = shaderCode.c_str();
    glShaderSource(shaderID, 1, &shaderCodeCStr, NULL);
    glCompileShader(shaderID);

    return shaderID;
}

void ShaderProgram::checkCompileErrors(unsigned shader, std::string shaderType)
{
    int success;
    char infoLog[512];
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << shaderType << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
}

void ShaderProgram::checkLinkErrors()
{
    int success;
    char infoLog[512];

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}