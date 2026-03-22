#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "Framework/Inc/Window.h"
#include "Framework/Inc/VideoLoader.h"
#include "Framework/Inc/FPSCounter.h"

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D videoTexture;
    void main()
    {
        FragColor = texture(videoTexture, TexCoord);
    }
)";

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
    float scaledWidth = scaleX * 1.0f;

    float vertices[] = {
        scaledWidth,  0.5f, 0.0f,  1.0f, 0.0f,
        scaledWidth, -0.5f, 0.0f,  1.0f, 1.0f,
       -scaledWidth, -0.5f, 0.0f,  0.0f, 1.0f,
       -scaledWidth,  0.5f, 0.0f,  0.0f, 0.0f
    };
    unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

    unsigned int VAO, VBO, EBO, texture;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenTextures(1, &texture);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "videoTexture"), 0);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

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

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame.data[0]);
        }

        window.PollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        window.SwapBuffers();
        renderFPSCounter.Tick();
        
        std::ostringstream title;
        title << "Frame Generation | Render FPS: " << std::fixed << std::setprecision(1) << renderFPSCounter.GetFPS()
              << " | Video FPS: " << videoFPSCounter.GetFPS();
        window.SetTitle(title.str());
    }

    videoLoader.Close();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProgram);

    return 0;
}
