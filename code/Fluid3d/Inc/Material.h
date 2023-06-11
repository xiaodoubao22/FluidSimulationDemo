#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

namespace Fluid3d {
	class Material {
	public:
		Material();
		~Material();

		void Create();
		void Destroy();
		void LoadTexures(std::string& albedoPath);

	public:
		GLuint mTexAlbedo = 0;
	};

}

#endif // !MATERIAL_H

