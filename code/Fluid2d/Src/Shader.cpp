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

int32_t Shader::BuildFromFile(std::string& vertPath, std::string& geomPath, std::string& fragPath) {
    std::string vertexCode, geometryCode, fragmentCode;
    std::ifstream vertShaderFile, geomShaderFile, fragShaderFile;
    std::stringstream vShaderStream, gShaderStream, fShaderStream;
    // ∂¡»° vertex shader
    vertShaderFile.open(vertPath);
    if (!vertShaderFile) {
        std::cout << "ERROR: Vertex shader file open failed" << std::endl;
        return -1;
    }
    vShaderStream << vertShaderFile.rdbuf();
    vertShaderFile.close();
    vertexCode = vShaderStream.str();
    const char* vShaderCode = vertexCode.c_str();
    // vertex shader
    unsigned int vertex;
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

    // ∂¡»° fragment Shader
    fragShaderFile.open(fragPath);
    if (!fragShaderFile) {
        std::cout << "ERROR: Fragment shader file open failed" << std::endl;
        return -1;
    }
    fShaderStream << fragShaderFile.rdbuf();
    fragShaderFile.close();
    fragmentCode = fShaderStream.str();
    const char* fShaderCode = fragmentCode.c_str();
    // fragment Shader
    unsigned int fragment;
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

    unsigned int geometry = 0;
    if (geomPath != std::string("")) {
        // ∂¡»° geometry Shader
        geomShaderFile.open(geomPath);
        if (!geomShaderFile) {
            std::cout << "ERROR: Geometry shader file open failed" << std::endl;
            return -1;
        }
        gShaderStream << geomShaderFile.rdbuf();
        geomShaderFile.close();
        geometryCode = gShaderStream.str();
        const char* gShaderCode = geometryCode.c_str();
        // geometry Shader
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        // check compile errors
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometry, 1024, NULL, infoLog);
            std::cout << "ERROR::GEOMETRY_SHADER_COMPILATION_ERROR: " << "\n" << infoLog << std::endl;
            return -1;
        }
    }
    
    // shader Program
    mId = glCreateProgram();
    glAttachShader(mId, vertex);
    glAttachShader(mId, fragment);
    if (geomPath != std::string("")) {
        glAttachShader(mId, geometry);
    }
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
    if (geomPath != std::string("")) {
        glDeleteShader(geometry);
    }
    std::cout << "success" << std::endl;

    return 0;
}

void Shader::Use() {
    glUseProgram(mId);
}

void Shader::UnUse() {
    glUseProgram(0);
}
