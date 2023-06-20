#include "Material.h"

#include "stb_image.h"
#include <iostream>

namespace Fluid3d {
	Material::Material() {
		
	}

	Material::~Material() {

	}

	void Material::Create() {
		glGenTextures(1, &mTexAlbedo);
		glGenTextures(1, &mTexRoughness);
	}

	void Material::Destroy() {
		if (mTexAlbedo != 0) {
			glDeleteTextures(1, &mTexAlbedo);
			glDeleteTextures(1, &mTexRoughness);
		}
	}

	void Material::LoadTextures(std::string& albedoPath, std::string& roughnessPath) {
		LoadTexture(mTexAlbedo, albedoPath);
		LoadTexture(mTexRoughness, roughnessPath);
		return;
	}

	void Material::LoadTexture(GLuint texId, std::string& path) {
		// 加载并生成纹理
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cout << "Failed to load texture albedo" << std::endl;
			stbi_image_free(data);
			return;
		}

		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		return;
	}

}