#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  

class Shader
{
public:
    unsigned int ID;
  
    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // Destructor
    ~Shader();
    // Aactivate the shader
    void use();
    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, bool transpose, const GLfloat* value) const;
};
  
#endif