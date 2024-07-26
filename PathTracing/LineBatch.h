#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Shader.h"

struct LineVertex
{
    glm::vec3 Position;
    glm::vec4 Color;
};

class LineBatch
{
public:
    LineBatch();
    ~LineBatch() noexcept;

    void FlushDraw();

    float Thickness = 1.0f;

    void DrawLine(glm::vec3 start, glm::vec3 end, const glm::mat4& transform, glm::vec4 color = glm::vec4(1.0f));

    void OnBeginScene(glm::mat4 projection);

private:
    LineVertex* m_Vertices;
    LineVertex* m_LastVertex;
    LineVertex* m_EndVertex;

    VertexArray m_VertexArray;
    std::shared_ptr<impl::VertexBuffer> m_VertexBuffer;
    glm::mat4 m_Projection;
    std::shared_ptr<Shader> m_Shader;
};

