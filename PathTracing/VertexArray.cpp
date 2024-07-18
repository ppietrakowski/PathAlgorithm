#include "VertexArray.h"
#include <cassert>
#include <glad/glad.h>

void VertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
    if (m_IndexBuffer)
    {
        DriverBeforeSetIndexBuffer(*indexBuffer);
    }

    m_IndexBuffer = indexBuffer;
    DriverSetIndexBuffer(*indexBuffer);
    m_NumIndices = indexBuffer->GetNumIndices();
}

VertexArray::VertexArray() :
    m_NumVertices(0),
    m_NumIndices(0)
{
    glCreateVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray() noexcept
{
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

struct GLVertexTypeMapping
{
    GLenum GlPrimitiveType;
    GLint NumComponents;
};

static inline GLVertexTypeMapping VertexTypeToGLMapping(EVertexType type)
{
    switch (type)
    {
    case EVertexType::int32_t:
        return {GL_INT, 1};
    case EVertexType::Float:
        return {GL_FLOAT, 1};
    case EVertexType::UnsignedInt:
        return {GL_UNSIGNED_INT, 1};
    case EVertexType::Vec2:
        return {GL_FLOAT, 2};
    case EVertexType::Vec3:
        return {GL_FLOAT, 3};
    case EVertexType::Vec4:
        return {GL_FLOAT, 4};
    case EVertexType::IntVec2:
        return {GL_INT, 2};
    case EVertexType::IntVec3:
        return {GL_INT, 3};
    case EVertexType::IntVec4:
        return {GL_INT, 4};
    case EVertexType::Last:
        break;
    default:
        break;
    }

    return {0, 0};
}


void VertexArray::DriverAddBuffer(impl::VertexBuffer& vertexBuffer) const
{
    Bind();
    vertexBuffer.Bind();

    impl::BufferLayout layout = vertexBuffer.GetBufferLayout();

    intptr_t offset = 0;
    GLuint attributeStartIndex = static_cast<GLuint>(GetNumVertexBuffers());

    for (auto vertexType : layout)
    {
        glEnableVertexAttribArray(attributeStartIndex);
        GLenum dataNormalized = GL_FALSE;
        auto mapping = VertexTypeToGLMapping(vertexType);
        assert(mapping.NumComponents > 0);

        GLsizei stride = layout.GetStride();

        if (mapping.GlPrimitiveType != GL_FLOAT)
        {
            glVertexAttribIPointer(attributeStartIndex, mapping.NumComponents, mapping.GlPrimitiveType,
                stride, reinterpret_cast<const void*>(offset));
        }
        else
        {
            glVertexAttribPointer(attributeStartIndex, mapping.NumComponents, mapping.GlPrimitiveType,
                dataNormalized, stride, reinterpret_cast<const void*>(offset));
        }

        offset += impl::GetSizeOfVertexType(vertexType);
        attributeStartIndex++;
    }
}

void VertexArray::DriverBeforeSetIndexBuffer(IndexBuffer& indexBuffer) const
{
    glBindVertexArray(m_RendererID);
    indexBuffer.Unbind();
}

void VertexArray::DriverSetIndexBuffer(IndexBuffer& indexBuffer) const
{
    glBindVertexArray(m_RendererID);
    indexBuffer.Bind();
}
