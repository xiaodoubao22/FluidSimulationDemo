#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>

namespace Glb {
    class Mesh {
    public:
        Mesh();
        ~Mesh();

        void Create();
        void Destroy();

        void SetVertices(const float* vertices, size_t size);
        void SetIndices(const unsigned int* indices, size_t count);

        void AddAttribute(GLuint index, GLint size, GLsizei stride, size_t offset);

        void Bind() const;
        void UnBind() const;

        void Draw(GLenum mode = GL_TRIANGLES) const;
        void DrawInstanced(GLsizei instanceCount, GLenum mode = GL_TRIANGLES) const;

        GLuint GetVAO() const;
        size_t GetIndexCount() const;

    private:
        GLuint mVAO = 0;
        GLuint mVBO = 0;
        GLuint mEBO = 0;
        size_t mIndexCount = 0;
    };
}

#endif
