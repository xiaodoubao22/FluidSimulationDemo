#include <Shader.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() {
	mId = 0;
}

Shader::~Shader() {
    glDeleteProgram(mId);
}

int32_t Shader::BuildFromFile(std::string& vertPath, std::string& fragPath) {
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vertShaderFile;
	std::ifstream fragShaderFile;
	vertShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // 打开
        vertShaderFile.open(vertPath);
        fragShaderFile.open(fragPath);
        // 读文件
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vertShaderFile.rdbuf();
        fShaderStream << fragShaderFile.rdbuf();
        // 关闭文件
        vertShaderFile.close();
        fragShaderFile.close();
        // 提取
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // check compile errors
    int success;
    char infoLog[1024];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER_COMPILATION_ERROR: " << "\n" << infoLog << std::endl;
        return -1;
    }
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // check compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER_COMPILATION_ERROR: " << "\n" << infoLog << std::endl;
        return -1;
    }
    // shader Program
    mId = glCreateProgram();
    glAttachShader(mId, vertex);
    glAttachShader(mId, fragment);
    glLinkProgram(mId);
    // check link errors
    glGetProgramiv(mId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(mId, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR:" << "\n" << infoLog << std::endl;
        return -1;
    }
    // delete the shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    std::cout << "success" << std::endl;

    return 0;
}

void Shader::Use() {
    glUseProgram(mId);
}

void Shader::UnUse() {
    glUseProgram(0);
}

void Shader::GetUniformParaId() {

}

void Shader::SetUniform() {

}
