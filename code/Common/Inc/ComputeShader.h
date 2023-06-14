#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace Glb {

	class ComputeShader {
	public:
		ComputeShader() = delete;
		ComputeShader(std::string name);
		~ComputeShader();

		int32_t BuildFromFile(std::string& compPath);
		int32_t BuildFromFiles(std::vector<std::string>& compPaths);
		void Use();
		void UnUse();
		GLuint GetId();

		void SetBool(const std::string& name, bool value);
		void SetInt(const std::string& name, int value);
		void SetUInt(const std::string& name, uint32_t value);
		void SetFloat(const std::string& name, float value);
		void SetVec2(const std::string& name, const glm::vec2& value);
		void SetVec2(const std::string& name, float x, float y);
		void SetVec3(const std::string& name, const glm::vec3& value);
		void SetVec3(const std::string& name, float x, float y, float z);
		void SetUVec3(const std::string& name, const glm::uvec3& value);
		void SetVec4(const std::string& name, const glm::vec4& value);
		void SetVec4(const std::string& name, float x, float y, float z, float w);
		void SetMat2(const std::string& name, const glm::mat2& mat);
		void SetMat3(const std::string& name, const glm::mat3& mat);
		void SetMat4(const std::string& name, const glm::mat4& mat);

	private:
		std::string mName;
		GLuint mId = 0;

	};
}

#endif // !COMPUTE_SHADER_H
