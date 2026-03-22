#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "Framework/Inc/Window.h"
#include "Framework/Inc/VideoLoader.h"
#include "Framework/Inc/FPSCounter.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

int main()
{
    Fw::Window window(1280, 720, "Frame Generation");

    Fw::VideoLoader videoLoader;
    std::string videoPath = "../resources/Videos/pubgTest.mp4";
    
    if (!videoLoader.Open(videoPath)) {
        std::cerr << "Failed to open video: " << videoPath << std::endl;
        return -1;
    }

    int width = videoLoader.GetWidth();
    int height = videoLoader.GetHeight();

    float aspectRatio = static_cast<float>(width) / height;
    float screenAspect = 1280.0f / 720.0f;
    float scaleX = aspectRatio / screenAspect;

    float vertices[] = {
         scaleX,  0.5f, 0.0f,  1.0f, 0.0f,
         scaleX, -0.5f, 0.0f,  1.0f, 1.0f,
        -scaleX, -0.5f, 0.0f,  0.0f, 1.0f,
        -scaleX,  0.5f, 0.0f,  0.0f, 0.0f
    };
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
    videoTexture.SetWrapS(GL_REPEAT);
    videoTexture.SetWrapT(GL_REPEAT);
    videoTexture.SetMinFilter(GL_LINEAR);
    videoTexture.SetMagFilter(GL_LINEAR);
    videoTexture.Allocate(width, height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
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
    videoShader.SetFloat("uScaleX", 1.0f);
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
            videoTexture.Update(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame.data[0]);
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
        
        std::ostringstream title;
        title << "Frame Generation | Render FPS: " << std::fixed << std::setprecision(1) << renderFPSCounter.GetFPS()
              << " | Video FPS: " << videoFPSCounter.GetFPS();
        window.SetTitle(title.str());
    }

    videoLoader.Close();

    return 0;
}
