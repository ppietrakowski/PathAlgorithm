#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <array>
#include <vector>

enum class EVertexType : uint8_t
{
    int32_t = 0,
    Float,
    UnsignedInt,
    Vec2,
    Vec3,
    Vec4,
    IntVec2,
    IntVec3,
    IntVec4,
    Last
};

template<typename T>
concept TVertexTraits = requires(const T&)
{
    {
        T::GetDataFormat()
    } -> std::convertible_to<std::span<const EVertexType>>;
};

template <typename T, TVertexTraits>
class TVertexBuffer;

namespace impl
{
    int32_t GetSizeOfVertexType(EVertexType vertexType);

    class BufferLayout
    {
    public:
        BufferLayout() = default;
        BufferLayout(const std::initializer_list<EVertexType>& dataFormat);
        BufferLayout(std::span<const EVertexType> dataFormat);
        BufferLayout(const BufferLayout& bufferLayout) = default;
        BufferLayout& operator=(const BufferLayout& bufferLayout) = default;

        int32_t GetStride() const
        {
            return m_Stride;
        }

        auto begin() const
        {
            return m_DataFormat.begin();
        }

        auto end() const
        {
            return m_DataFormat.end();
        }

    private:
        std::vector<EVertexType> m_DataFormat;
        int32_t m_Stride = 0;
    };

    class VertexBuffer
    {
        template <typename T, TVertexTraits Traits>
        friend class ::TVertexBuffer;

    public:

        ~VertexBuffer() noexcept;

    public:
        void UpdateBuffer(const void* data, int32_t size, int32_t offset);
        uint32_t GetNativeHandle() const;

        void Bind();
        void Unbind();
        BufferLayout GetBufferLayout() const;
        int32_t GetSizeBytes() const;

    public:
        static size_t GetIndexBufferAllocationSize()
        {
            return s_TotalAllocatedBufferSize;
        }

    private:
        uint32_t m_RendererID;
        int32_t m_Size;
        std::vector<EVertexType> m_DataFormat;
        static size_t s_TotalAllocatedBufferSize;

    protected:
        VertexBuffer(const void* data, int32_t size, bool dynamic, std::span<const EVertexType> dataFormat);
    };
}

enum EForceNoInit
{
    ForceNoInit
};


template <typename VertexType, TVertexTraits VertexTraits>
class TVertexBuffer
{
    friend class VertexArray;

public:
    using SelfClass = TVertexBuffer<VertexType, VertexTraits>;

    TVertexBuffer(std::span<const VertexType> vertices, bool dynamic = false)
    {
        struct VBContruct : public impl::VertexBuffer
        {
            typedef impl::VertexBuffer Super;

            VBContruct(const void* data, int32_t size, bool dynamic, std::span<const EVertexType> dataFormat) :
                Super(data, size, dynamic, dataFormat)
            {
            }
        };

        m_VertexBufferImpl = std::make_shared<VBContruct>(vertices.data(), (int32_t)vertices.size_bytes(), dynamic, VertexTraits::GetDataFormat());
    }

    TVertexBuffer(int32_t capacity)
    {
        struct VBContruct : public impl::VertexBuffer
        {
            typedef impl::VertexBuffer Super;

            VBContruct(int32_t size) :
                Super(nullptr, size, true, VertexTraits::GetDataFormat())
            {
            }
        };

        m_VertexBufferImpl = std::make_shared<VBContruct>(capacity * sizeof(VertexType));
    }

    TVertexBuffer(const SelfClass&) = default;
    TVertexBuffer& operator=(const SelfClass&) = default;

    TVertexBuffer(SelfClass&&) noexcept = default;
    TVertexBuffer& operator=(SelfClass&&) noexcept = default;

    void UpdateBuffer(std::span<const VertexType> vertices, int32_t offset = 0)
    {
        m_VertexBufferImpl->UpdateBuffer(vertices.data(), (int32_t)vertices.size_bytes(), offset * sizeof(VertexType));
    }

    uint32_t GetNativeHandle() const
    {
        return m_VertexBufferImpl->GetNativeHandle();
    }

    impl::BufferLayout GetBufferLayout() const
    {
        return m_VertexBufferImpl->GetBufferLayout();
    }

    int32_t GetSizeBytes() const
    {
        return m_VertexBufferImpl->GetSizeBytes();
    }

    int32_t GetNumElements() const
    {
        return m_VertexBufferImpl->GetSizeBytes() / sizeof(VertexType);
    }

    void Bind()
    {
        m_VertexBufferImpl->Bind();
    }

    void Unbind()
    {
        m_VertexBufferImpl->Unbind();
    }

private:
    std::shared_ptr<impl::VertexBuffer> m_VertexBufferImpl;
};

class IndexBuffer
{
public:

    static [[nodiscard]] std::shared_ptr<IndexBuffer> Create(std::span<const uint32_t> data, bool dynamic = false);
    static [[nodiscard]] std::shared_ptr<IndexBuffer> CreateEmptyDynamic(int32_t maxNumIndices);
    virtual ~IndexBuffer() noexcept;

public:
    void Bind();
    void Unbind();
    void Update(std::span<const uint32_t> data, int32_t offset);
    int32_t GetNumIndices() const;
    uint32_t GetBufferID() const;

public:
    static size_t GetIndexBufferAllocationSize();

private:
    static size_t s_TotalAllocatedBufferSize;

    uint32_t m_RendererID;
    int32_t m_NumIndices;

protected:
    IndexBuffer(std::span<const uint32_t> data, bool dynamic);
    IndexBuffer(int32_t maxNumIndices);

private:
    static uint32_t GenerateRendererID(const uint32_t* data, int32_t size, bool dynamic);
};