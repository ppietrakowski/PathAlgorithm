#pragma once

#include <cstdint>
#include <span>

class UniformBuffer
{
public:
    UniformBuffer(int32_t maxSizeBytes);
    ~UniformBuffer() noexcept;

public:
    void Bind(int32_t bindingId) const;
    void UpdateBuffer(const void* data, int32_t size, int32_t offset);

private:
    uint32_t m_RendererID{0};
    int32_t m_SizeBytes{0};
};

template <typename ElementType>
class TUniformBuffer
{
public:
    TUniformBuffer(int32_t maxNumElements) :
        m_UniformBuffer(maxNumElements * sizeof(ElementType))
    {
    }

    void Bind(int32_t bindingId) const
    {
        m_UniformBuffer.Bind(bindingId);
    }

    void UpdateBuffer(std::span<const ElementType> data, int32_t offset = 0)
    {
        m_UniformBuffer.UpdateBuffer(data.data(), (int32_t)data.size_bytes(), offset * sizeof(ElementType));
    }

    void UpdateElement(const ElementType& element, int32_t offset = 0)
    {
        m_UniformBuffer.UpdateBuffer(&element, sizeof(element), offset * sizeof(ElementType));
    }

    const UniformBuffer& GetUniformBuffer() const
    {
        return m_UniformBuffer;
    }

    UniformBuffer& GetUniformBuffer()
    {
        return m_UniformBuffer;
    }

private:
    UniformBuffer m_UniformBuffer;
};