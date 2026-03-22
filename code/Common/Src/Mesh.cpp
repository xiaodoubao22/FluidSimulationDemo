#include "Mesh.h"

namespace Glb {
    Mesh::Mesh() : mVAO(0), mVBO(0), mEBO(0), mIndexCount(0) {
    }

    Mesh::~Mesh() {
        Destroy();
    }

    void Mesh::Create() {
        if (mVAO != 0) {
            Destroy();
        }
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);
        glGenBuffers(1, &mEBO);
    }

    void Mesh::Destroy() {
        if (mVAO != 0) {
            glDeleteVertexArrays(1, &mVAO);
            glDeleteBuffers(1, &mVBO);
            glDeleteBuffers(1, &mEBO);
            mVAO = 0;
            mVBO = 0;
            mEBO = 0;
            mIndexCount = 0;
        }
    }

    void Mesh::SetVertices(const float* vertices, size_t size) {
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

    void Mesh::SetIndices(const unsigned int* indices, size_t count) {
        mIndexCount = count;
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

    void Mesh::AddAttribute(GLuint index, GLint size, GLsizei stride, size_t offset) {
        glBindVertexArray(mVAO);
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        glEnableVertexAttribArray(index);
        glBindVertexArray(0);
    }

    void Mesh::Bind() const {
        glBindVertexArray(mVAO);
    }

    void Mesh::UnBind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw(GLenum mode) const {
        glBindVertexArray(mVAO);
        glDrawElements(mode, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Mesh::DrawInstanced(GLsizei instanceCount, GLenum mode) const {
        glBindVertexArray(mVAO);
        glDrawElementsInstanced(mode, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, 0, instanceCount);
        glBindVertexArray(0);
    }

    GLuint Mesh::GetVAO() const {
        return mVAO;
    }

    size_t Mesh::GetIndexCount() const {
        return mIndexCount;
    }
}
