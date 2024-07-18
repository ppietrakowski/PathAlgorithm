#include "UniformBuffer.h"
#include <glad/glad.h>
#include <cassert>

UniformBuffer::UniformBuffer(int32_t maxSizeBytes) :
    m_SizeBytes{maxSizeBytes}
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
    glBufferData(GL_UNIFORM_BUFFER, maxSizeBytes, nullptr, GL_DYNAMIC_DRAW);
}

UniformBuffer::~UniformBuffer() noexcept
{
    glDeleteBuffers(1, &m_RendererID);
}

void UniformBuffer::Bind(int32_t bindingId) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingId, m_RendererID);
}

void UniformBuffer::UpdateBuffer(const void* data, int32_t size, int32_t offset)
{
    assert(size >= 0 && size <= m_SizeBytes && "Size would overflow");
    assert(offset >= 0 && offset <= m_SizeBytes && "Offset would overflow");

    glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}
