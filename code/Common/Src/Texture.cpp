#include "Texture.h"

namespace Glb {
    Texture::Texture() : mId(0), mWidth(0), mHeight(0) {
    }

    Texture::~Texture() {
        Destroy();
    }

    void Texture::Create() {
        if (mId != 0) {
            Destroy();
        }
        glGenTextures(1, &mId);
    }

    void Texture::Destroy() {
        if (mId != 0) {
            glDeleteTextures(1, &mId);
            mId = 0;
            mWidth = 0;
            mHeight = 0;
        }
    }

    void Texture::Bind() const {
        glBindTexture(GL_TEXTURE_2D, mId);
    }

    void Texture::UnBind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::Allocate(int width, int height, GLenum format, GLenum type, const void* data) {
        mWidth = width;
        mHeight = height;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);
    }

    void Texture::Update(int xoffset, int yoffset, int width, int height, GLenum format, GLenum type, const void* data) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, format, type, data);
    }

    void Texture::SetWrapS(GLenum mode) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    }

    void Texture::SetWrapT(GLenum mode) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
    }

    void Texture::SetMinFilter(GLenum mode) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
    }

    void Texture::SetMagFilter(GLenum mode) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
    }

    GLuint Texture::GetId() const {
        return mId;
    }

    int Texture::GetWidth() const {
        return mWidth;
    }

    int Texture::GetHeight() const {
        return mHeight;
    }
}
