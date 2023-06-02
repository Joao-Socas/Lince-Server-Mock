#include "Shader.hpp"
#include <string.h>

ShaderProgram::ShaderProgram(VertexShader vertexShader, FragmentShader fragmentShader)
{
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader.ID);
    glAttachShader(ID, fragmentShader.ID);
    glLinkProgram(ID);
    checkCompileErrors();
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader.ID);
    glDeleteShader(fragmentShader.ID);
}


void ShaderProgram::use()
{
    glUseProgram(ID);
}


void ShaderProgram::checkCompileErrors()
{
    int success;
    char infoLog[1024];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR"<< "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}

Shader::Shader(const char* Path)
{
    std::string SshaderCode;
    std::ifstream shaderFile;

    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        shaderFile.open(Path);
        std::stringstream shaderStream;
        // read file's buffer contents into streams
        shaderStream << shaderFile.rdbuf();

        // close file handlers
        shaderFile.close();

        // convert stream into string
        SshaderCode = shaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    shaderCode = new char[SshaderCode.length() + 1];
    strcpy_s(shaderCode, SshaderCode.length() + 1, SshaderCode.c_str());
}



void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}

void VertexShader::Compile()
{
    ID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(ID, 1, &shaderCode, NULL);
    glCompileShader(ID);
    checkCompileErrors(ID, "VERTEX");
}

void FragmentShader::Compile()
{
    ID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ID, 1, &shaderCode, NULL);
    glCompileShader(ID);
    checkCompileErrors(ID, "FRAGMENT");
}
