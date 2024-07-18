#pragma once

#include "Buffers.h"

class VertexArray
{
public:
    VertexArray();
    ~VertexArray() noexcept;

public:
    void Bind() const;
    void Unbind() const;

    template <typename VertexType, typename VertexTraits>
    void AddBuffer(const TVertexBuffer<VertexType, VertexTraits>& vertexBuffer)
    {
        AddBufferImpl(vertexBuffer.m_VertexBufferImpl);
    }

    void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

    int32_t GetNumIndices() const
    {
        return m_NumIndices;
    }

    int32_t GetNumVertices() const
    {
        return m_NumVertices;
    }

    size_t GetNumVertexBuffers() const
    {
        return m_VertexBuffers.size();
    }

    std::shared_ptr<impl::VertexBuffer> GetVertexBuffer(size_t index)
    {
        return m_VertexBuffers[index];
    }

private:
    void DriverAddBuffer(impl::VertexBuffer& vertexBuffer) const;
    void DriverBeforeSetIndexBuffer(IndexBuffer& indexBuffer) const;
    void DriverSetIndexBuffer(IndexBuffer& indexBuffer) const;

private:
    std::vector<std::shared_ptr<impl::VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    int32_t m_NumVertices;
    int32_t m_NumIndices;
    uint32_t m_RendererID;

    void AddBufferImpl(std::shared_ptr<impl::VertexBuffer> vertexBuffer)
    {
        if (m_VertexBuffers.empty())
        {
            m_NumVertices = vertexBuffer->GetSizeBytes() / vertexBuffer->GetBufferLayout().GetStride();
        }

        DriverAddBuffer(*vertexBuffer);
        m_VertexBuffers.emplace_back(vertexBuffer);
    }

    std::shared_ptr<impl::VertexBuffer> GetVertexBufferAt(size_t index) const
    {
        return m_VertexBuffers[index];
    }

    std::shared_ptr<IndexBuffer> GetIndexBuffer() const
    {
        return m_IndexBuffer;
    }
};

