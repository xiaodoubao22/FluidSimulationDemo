#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

namespace Glb {
    class Shader {
    public:
        Shader();
        ~Shader();

        int32_t BuildFromFile(std::string& vertPath, std::string& fragPath, std::string& geomPath = std::string(""));
        void Use();
        void UnUse();
        GLuint GetId();

        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
        void SetVec2(const std::string& name, const glm::vec2& value);
        void SetVec2(const std::string& name, float x, float y);
        void SetVec3(const std::string& name, const glm::vec3& value);
        void SetVec3(const std::string& name, float x, float y, float z);
        void SetVec4(const std::string& name, const glm::vec4& value);
        void SetVec4(const std::string& name, float x, float y, float z, float w);
        void SetMat2(const std::string& name, const glm::mat2& mat);
        void SetMat3(const std::string& name, const glm::mat3& mat);
        void SetMat4(const std::string& name, const glm::mat4& mat);

    private:
        GLuint mId = 0;

    };
}



#endif // !SHADER_H
