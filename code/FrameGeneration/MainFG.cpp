#include <iostream>
#include <chrono>
#include "Framework/Inc/Window.h"
#include "Framework/Inc/VideoLoader.h"
#include "Framework/Inc/FPSCounter.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

void GenerateQuadVertices(float videoAspect, float screenAspect, float* vertices) {
    float halfWidth, halfHeight;
    if (videoAspect > screenAspect) {
        halfHeight = 1.0f;
        halfWidth = screenAspect * halfHeight / videoAspect;
    } else {
        halfWidth = 1.0f;
        halfHeight = halfWidth * videoAspect / screenAspect;
    }

    vertices[0]  =  halfWidth; vertices[1]  =  halfHeight; vertices[2]  = 0.0f; vertices[3]  = 1.0f; vertices[4]  = 0.0f;
    vertices[5]  =  halfWidth; vertices[6]  = -halfHeight; vertices[7]  = 0.0f; vertices[8]  = 1.0f; vertices[9]  = 1.0f;
    vertices[10] = -halfWidth; vertices[11] = -halfHeight; vertices[12] = 0.0f; vertices[13] = 0.0f; vertices[14] = 1.0f;
    vertices[15] = -halfWidth; vertices[16] =  halfHeight; vertices[17] = 0.0f; vertices[18] = 0.0f; vertices[19] = 0.0f;
}

int main()
{
    Fw::Window window(1280, 720, "Frame Generation");

    Fw::VideoLoader videoLoader;
    std::string videoPath = "../resources/Videos/pubgTest.mp4";
    
    if (!videoLoader.Open(videoPath)) {
        std::cerr << "Failed to open video: " << videoPath << std::endl;
        return -1;
    }

    int videoWidth = videoLoader.GetWidth();
    int videoHeight = videoLoader.GetHeight();
    float videoAspect = static_cast<float>(videoWidth) / videoHeight;
    float screenAspect = 1280.0f / 720.0f;

    float vertices[20];
    GenerateQuadVertices(videoAspect, screenAspect, vertices);
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

    Glb::Mesh quadMesh;
    quadMesh.Create();
    quadMesh.SetVertices(vertices, sizeof(vertices));
    quadMesh.SetIndices(indices, 6);
    quadMesh.AddAttribute(0, 3, 5 * sizeof(float), 0);
    quadMesh.AddAttribute(1, 2, 5 * sizeof(float), 3 * sizeof(float));

    Glb::Texture videoTexture;
    videoTexture.Create();
    videoTexture.Bind();
    videoTexture.SetWrapS(GL_CLAMP_TO_BORDER);
    videoTexture.SetWrapT(GL_CLAMP_TO_BORDER);
    videoTexture.SetMinFilter(GL_LINEAR);
    videoTexture.SetMagFilter(GL_LINEAR);
    videoTexture.Allocate(videoWidth, videoHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    videoTexture.UnBind();

    Glb::Shader videoShader;
    std::string vertPath = "../code/FrameGeneration/Shaders/VideoFrame.vert";
    std::string fragPath = "../code/FrameGeneration/Shaders/VideoFrame.frag";
    if (videoShader.BuildFromFile(vertPath, fragPath) != 0) {
        std::cerr << "Failed to build shader" << std::endl;
        return -1;
    }
    videoShader.Use();
    videoShader.SetInt("videoTexture", 0);
    videoShader.UnUse();

    Fw::VideoFrame frame;
    Fw::FPSCounter renderFPSCounter;
    Fw::FPSCounter videoFPSCounter;
    double frameInterval = 1.0 / videoLoader.GetFrameRate();
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (!window.ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(currentTime - lastFrameTime).count();
        
        if (elapsed >= frameInterval) {
            if (!videoLoader.ReadFrame(frame)) {
                videoLoader.Close();
                videoLoader.Open(videoPath);
                lastFrameTime = std::chrono::high_resolution_clock::now();
                std::cout << "Video looped" << std::endl;
                continue;
            }
            lastFrameTime = currentTime;
            videoFPSCounter.Tick();

            std::cout << "Video FPS: " << videoFPSCounter.GetFPS() 
                      << " | Render FPS: " << renderFPSCounter.GetFPS() 
                      << " | Frame: " << videoLoader.GetCurrentFrame() << " / " << videoLoader.GetTotalFrames() << "\r" << std::flush;

            videoTexture.Bind();
            videoTexture.Update(0, 0, videoWidth, videoHeight, GL_RGB, GL_UNSIGNED_BYTE, frame.data[0]);
            videoTexture.UnBind();
        }

        window.PollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        videoShader.Use();
        videoTexture.Bind();
        quadMesh.Draw();
        videoTexture.UnBind();

        window.SwapBuffers();
        renderFPSCounter.Tick();
        window.SetTitle(renderFPSCounter.GetFPS(), videoFPSCounter.GetFPS());
    }

    videoLoader.Close();

    return 0;
}
