#ifndef FLUID_SHADOW_MAP_H
#define FLUID_SHADOW_MAP_H

#include <glad/glad.h>
#include "Shader.h"

namespace Fluid3d {
	struct PointLight {
		glm::vec3 pos;
		glm::vec3 dir;
		float_t fovy;
		float_t aspect;
	};

	class FluidShadowMap {
	public:
		FluidShadowMap();
		~FluidShadowMap();

		void Create(int32_t w, int32_t h, PointLight& light);
		void Destroy();
		void Update(GLuint vaoParticals, int32_t particalNum);
		GLuint GetTextureId();

	public:
		Glb::Shader* mPointSpriteZValue = nullptr;

		int32_t mWidth = 1024;
		int32_t mHeight = 1024;

		GLuint mFbo = 0;
		GLuint mTextureZBuffer = 0;
		GLuint mDepthStencil = 0;

		glm::mat4 mLightView;
		glm::mat4 mLightProjection;
		glm::vec3 mLightViewUp;
		glm::vec3 mLightViewRight;
		glm::vec3 mLightViewFront;
	};
}


#endif // !FLUID_SHADOW_MAP_H


