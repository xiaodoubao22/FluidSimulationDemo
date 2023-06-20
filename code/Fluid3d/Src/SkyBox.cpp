#include "SkyBox.h"
#include "stb_image.h"

#include <iostream>

namespace Fluid3d {

    SkyBox::SkyBox() {
        mShader = new Glb::Shader();
    }

    SkyBox::~SkyBox() {
        delete mShader;
    }

    void SkyBox::Create() {
        glGenTextures(1, &mId);
    }

    void SkyBox::Destroy() {
        glDeleteTextures(1, &mId);
    }

    int32_t SkyBox::LoadImages(std::vector<std::string> paths) {
        if (paths.size() < 6 || mId == 0) {
            return -1;
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, mId);

        int width, height, nrChannels;
        unsigned char* data = nullptr;
        for (unsigned int i = 0; i < paths.size(); i++)
        {
            data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
            if (!data) {
                std::cout << "Cubemap texture failed to load at path: " << paths[i] << std::endl;
                stbi_image_free(data);
                continue;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            data = nullptr;
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        return 0;
    }

    void SkyBox::BuildShader() {
        std::string vertPath = "../code/Fluid3d/Shaders/SkyBox.vert";
        std::string fragPath = "../code/Fluid3d/Shaders/SkyBox.frag";
        mShader->BuildFromFile(vertPath, fragPath);
        mShader->Use();
        mShader->SetInt("skybox", 0);
        mShader->UnUse();
    }

    void SkyBox::Draw(GLFWwindow* window, GLuint nullVao, glm::mat4 view, glm::mat4 proj) {
        glDepthMask(GL_FALSE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        
        mShader->Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mId);
        mShader->SetMat4("view", glm::mat4(glm::mat3(view)));
        mShader->SetMat4("projection", proj);
        glBindVertexArray(nullVao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        mShader->UnUse();

        glDepthMask(GL_TRUE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    GLuint SkyBox::GetId() {
        return mId;
    }

}