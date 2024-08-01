#include "LineBatch.h"
#include <string>
#include <glad/glad.h>

static constexpr std::string_view s_VertexShader = R"(
#version 430 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;

uniform mat4 u_Projection;

out vec4 OutFragColor;

void main()
{
    gl_Position = vec4((u_Projection * vec4(a_Position, 1)).xyz, 1);
    OutFragColor = a_Color;
}
)";

static constexpr std::string_view s_FragmentShader = R"(
#version 430 core

out vec4 FragColor;
in vec4 OutFragColor;

void main()
{
    FragColor = OutFragColor;
}
)";

static constexpr size_t MaxNumBatchedLines = 400;

struct VerticesTrait
{
    static std::array<EVertexType, 2> GetDataFormat()
    {
        return {EVertexType::Vec3, EVertexType::Vec4};
    }
};

LineBatch::LineBatch():
    m_Vertices(new LineVertex[MaxNumBatchedLines]),
    m_LastVertex(m_Vertices),
    m_EndVertex(m_Vertices + MaxNumBatchedLines),
    m_Projection(1.0f)
{
    m_VertexArray.AddBuffer(TVertexBuffer<LineVertex, VerticesTrait>{MaxNumBatchedLines});

    m_VertexBuffer = m_VertexArray.GetVertexBuffer(0);
    ShaderSource source;
    source.VertexShaderSource = s_VertexShader;
    source.FragmentShaderSource = s_FragmentShader;

    if (!Shader::Create("default", source, m_Shader))
    {
        std::exit(EXIT_FAILURE);
    }

    m_Shader->Bind();
    m_Shader->SetMat4("u_Projection", m_Projection);
}

LineBatch::~LineBatch() noexcept
{
    delete[] m_Vertices;
}

void LineBatch::FlushDraw()
{
    if (m_LastVertex == m_Vertices)
    {
        return;
    }

    m_VertexBuffer->UpdateBuffer(m_Vertices, static_cast<int>((m_LastVertex - m_Vertices) *sizeof(LineVertex)), 0);

    m_VertexArray.Bind();
    glLineWidth(Thickness);

    m_Shader->Bind();

    m_Shader->SetMat4("u_Projection", m_Projection);
    glDrawArrays(GL_LINES, 0, m_LastVertex - m_Vertices);

    m_LastVertex = m_Vertices;
}

void LineBatch::DrawLine(glm::vec3 start, glm::vec3 end, const glm::mat4& transform, glm::vec4 color)
{
    if (m_LastVertex == m_EndVertex)
    {
        FlushDraw();
    }

    LineVertex vertex;
    vertex.Color = color;
    vertex.Position = transform * glm::vec4(start, 1.0f);

    *m_LastVertex++ = vertex;
    vertex.Position = transform * glm::vec4(end, 1.0f);
    *m_LastVertex++ = vertex;
}

void LineBatch::OnBeginScene(glm::mat4 projection)
{
    m_Projection = projection;
}
