#ifndef FLUID_SHADOW_MAP_H
#define FLUID_SHADOW_MAP_H

#include <glad/glad.h>
#include "Shader.h"
#include "ComputeShader.h"
#include "DepthFilter.h"
#include "RenderCamera.h"

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

		void SetImageSize(int32_t w, int32_t h);
		void SetLightInfo(PointLight& light);
		void SetIor(float_t ior);

		void Init();
		void Destroy();
		void Update(GLuint vaoParticals, int32_t particalNum, DepthFilter* depthFilter);
		void DrawCaustic(RenderCamera* camera, GLuint vaoNull, const glm::mat4& model);
		GLuint GetShadowMap();
		GLuint GetCausticMap();

	private:
		void CreateShaders();
		void CreateBuffers(int32_t w, int32_t h);
		void InitIntrinsic();

	public:
		Glb::Shader* mPointSpriteZValue = nullptr;
		Glb::Shader* mCausticMap = nullptr;

		int32_t mWidth = 1024;
		int32_t mHeight = 1024;

		GLuint mFbo = 0;
		GLuint mFboCaustic = 0;
		GLuint mTextureZBuffer = 0;
		GLuint mTextureTempZBuffer = 0;
		GLuint mTextureCausticMap = 0;
		GLuint mDepthStencil = 0;

		GLuint mZBufferA = 0;
		GLuint mZBufferB = 0;

		glm::mat4 mLightView;
		glm::mat4 mLightProjection;
		glm::vec3 mLightViewUp;
		glm::vec3 mLightViewRight;
		glm::vec3 mLightViewFront;

		float_t mIor = 1.0;
	};
}


#endif // !FLUID_SHADOW_MAP_H


