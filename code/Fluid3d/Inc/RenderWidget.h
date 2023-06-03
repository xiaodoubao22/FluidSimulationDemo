#ifndef RENDER_WIDGET_H
#define RENDER_WIDGET_H


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>

#include "ComputeShader.h"
#include "ParticalSystem3d.h"
#include "RenderCamera.h"

namespace Fluid3d {
    class RenderWidget
    {
    public:
        RenderWidget();
        ~RenderWidget();

        int32_t Init();

        // 上传、读取数据
        void UploadUniforms(Fluid3d::ParticalSystem3D& ps);
        void UploadParticalInfo(Fluid3d::ParticalSystem3D& ps);
        void DumpParticalInfo(Fluid3d::ParticalSystem3D& ps);

        // 求解、渲染
        void SolveParticals();
        void Update();
        
        // window
        bool ShouldClose();
        void ProcessInput();
        void PollEvents();

    private:
        static void ResizeCallback(GLFWwindow* window, int width, int height);
        static void CursorPosCallBack(GLFWwindow* window, double xpos, double ypos);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    private:
        bool CreateWindow();
        void UpdateFPS();
        void BuildShaders();
        void GenerateBuffers();
        void GenerateTextures();
        void MakeVaoParticals();
        void MakeVaoCoord();
        void DrawParticals();
        int32_t Destroy();

    private:
        // window
        GLFWwindow* mWindow = nullptr;
        int mWindowWidth = 1000;
        int mWindowHeight = 1000;

        // camera
        RenderCamera mCamera;
        bool mFirstMouseFlag = true;
        float mLastX;
        float mLastY;
        bool mLeftPressFlag = false;
        bool mRightPressFlag = false;

        // shaders
        Glb::Shader* mScreenQuad = nullptr;
        Glb::Shader* mDrawColor3d = nullptr;
        Glb::ComputeShader* mComputeShader = nullptr;

        // vao
        GLuint mVaoNull = 0;
        GLuint mVaoParticals = 0;
        GLuint mVaoCoord = 0;

        // buffers
        GLuint mCoordVertBuffer = 0;
        GLuint mBufferParticals = 0;
        GLuint mBufferBlocks = 0;

        // texures
        GLuint mTestTexture = 0;
        GLuint mTexKernelBuffer = 0;

        int mParticalNum = 0;
        float mUpdateTime = 0.0f;
        float updateTitleTime = 0.0f;
        float frameCount = 0.0f;
    };
}

#endif // !RENDER_WIDGET_H

