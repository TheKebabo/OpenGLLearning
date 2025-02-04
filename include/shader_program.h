#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <string>
#include <sstream>

class ShaderProgram
{
public:
    unsigned ID;

    ShaderProgram() {}
    // Destructor
    ~ShaderProgram();
    // Activate the shader program
    void use();
    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, bool transpose, const GLfloat* value) const;
protected:
    unsigned readAndCompileShaderFile(const char* shaderPath, unsigned& shaderID);
    void checkCompileErrors(unsigned shader, std::string shaderType);
    void checkLinkErrors();
};

#endif