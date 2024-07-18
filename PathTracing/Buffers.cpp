#include "Buffers.h"
#include <glad/glad.h>
#include <cassert>

namespace impl
{
    size_t VertexBuffer::s_TotalAllocatedBufferSize = 0;

    int32_t GetSizeOfVertexType(EVertexType vertexType)
    {
        switch (vertexType)
        {
        case EVertexType::int32_t:
            return sizeof(int32_t);
        case EVertexType::Float:
            return sizeof(float);
        case EVertexType::UnsignedInt:
            return sizeof(uint32_t);
        case EVertexType::Vec2:
            return sizeof(float) * 2;
        case EVertexType::Vec3:
            return sizeof(float) * 3;
        case EVertexType::Vec4:
            return sizeof(float) * 4;
        case EVertexType::IntVec2:
            return sizeof(int32_t) * 2;
        case EVertexType::IntVec3:
            return sizeof(int32_t) * 3;
        case EVertexType::IntVec4:
            return sizeof(int32_t) * 4;
        default:
            break;
        }

        return 0;
    }

    BufferLayout::BufferLayout(const std::initializer_list<EVertexType>& dataFormat) :
        m_DataFormat{dataFormat}
    {
        for (auto vertexType : dataFormat)
        {
            m_Stride += GetSizeOfVertexType(vertexType);
        }
    }

    BufferLayout::BufferLayout(std::span<const EVertexType> dataFormat) :
        m_DataFormat{dataFormat.begin(), dataFormat.end()}
    {
        for (auto vertexType : dataFormat)
        {
            m_Stride += GetSizeOfVertexType(vertexType);
        }
    }

    VertexBuffer::VertexBuffer(const void* data, int32_t size, bool dynamic, std::span<const EVertexType> dataFormat) :
        m_DataFormat{dataFormat.begin(), dataFormat.end()},
        m_Size{size}
    {
        glBindVertexArray(0);

        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        s_TotalAllocatedBufferSize += size;
    }

    VertexBuffer::~VertexBuffer() noexcept
    {
        glDeleteBuffers(1, &m_RendererID);
        s_TotalAllocatedBufferSize -= m_Size;
    }

    void VertexBuffer::UpdateBuffer(const void* data, int32_t size, int32_t offset)
    {
        assert(size <= size);
        assert(offset >= 0 && offset <= size);

        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    }

    uint32_t VertexBuffer::GetNativeHandle() const
    {
        return m_RendererID;
    }

    void VertexBuffer::Bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::Unbind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    impl::BufferLayout VertexBuffer::GetBufferLayout() const
    {
        return impl::BufferLayout{m_DataFormat};
    }

    int32_t VertexBuffer::GetSizeBytes() const
    {
        return m_Size;
    }
}

size_t IndexBuffer::s_TotalAllocatedBufferSize = 0;

class IndexBufferConstruct : public IndexBuffer
{
    typedef IndexBuffer Super;

public:
    IndexBufferConstruct(std::span<const uint32_t> data, bool dynamic) :
        Super(data, dynamic)
    {
    }

    IndexBufferConstruct(int32_t maxNumIndices) :
        Super(maxNumIndices)
    {
    }

    ~IndexBufferConstruct() = default;
};

std::shared_ptr<IndexBuffer> IndexBuffer::Create(std::span<const uint32_t> data, bool dynamic)
{
    return std::make_shared<IndexBufferConstruct>(data, dynamic);
}

std::shared_ptr<IndexBuffer> IndexBuffer::CreateEmptyDynamic(int32_t maxNumIndices)
{
    return std::make_shared<IndexBufferConstruct>(maxNumIndices);
}


IndexBuffer::IndexBuffer(std::span<const uint32_t> data, bool dynamic) :
    m_NumIndices{(int32_t)data.size()}
{
    m_RendererID = GenerateRendererID(data.data(), m_NumIndices, dynamic);
    s_TotalAllocatedBufferSize += m_NumIndices * sizeof(uint32_t);
}

IndexBuffer::IndexBuffer(int32_t maxNumIndices) :
    m_NumIndices(maxNumIndices)
{
    m_RendererID = GenerateRendererID(nullptr, maxNumIndices, true);
    s_TotalAllocatedBufferSize += m_NumIndices * sizeof(uint32_t);
}

IndexBuffer::~IndexBuffer() noexcept
{
    glDeleteBuffers(1, &m_RendererID);
    s_TotalAllocatedBufferSize -= m_NumIndices * sizeof(uint32_t);
}

void IndexBuffer::Bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::Update(std::span<const uint32_t> data, int32_t offset)
{
    assert(data.size() <= m_NumIndices);
    assert(offset >= 0 && offset <= m_NumIndices);

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(uint32_t), (int32_t)data.size_bytes(), data.data());
}

int32_t IndexBuffer::GetNumIndices() const
{
    return m_NumIndices;
}

uint32_t IndexBuffer::GetBufferID() const
{
    return m_RendererID;
}

uint32_t IndexBuffer::GenerateRendererID(const uint32_t* data, int32_t size, bool dynamic)
{
    GLuint rendererID;

    glBindVertexArray(0);
    glCreateBuffers(1, &rendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint32_t), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    return rendererID;
}

size_t IndexBuffer::GetIndexBufferAllocationSize()
{
    return s_TotalAllocatedBufferSize;
}
