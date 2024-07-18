#include "RectRenderer.h"

#include "Buffers.h"
#include "Shader.h"
#include "VertexArray.h"
#include "UniformBuffer.h"

#include <vector>
#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

static constexpr std::string_view s_VertexShader = R"(
#version 430 core

layout (location = 0) in vec3 a_Position;

uniform mat4 u_Projection;

layout(std140, binding=0) uniform Transforms {
    mat4 u_Transforms[400];
};

out flat int OutColorIndex;

void main()
{
    gl_Position = vec4((u_Projection * u_Transforms[gl_InstanceID] * vec4(a_Position, 1)).xyz, 1);
    OutColorIndex = gl_InstanceID;
}
)";

static constexpr std::string_view s_FragmentShader = R"(
#version 430 core

out vec4 FragColor;

layout(std140, binding=0) uniform Colors {
    vec4 u_Colors[400];
};

in flat int OutColorIndex;


void main()
{
    FragColor = u_Colors[OutColorIndex];
}
)";

static constexpr size_t MaxNumBatchedColors = 400;

class RectRenderer::RectRendererImpl
{
public:
    RectRendererImpl():
        m_TransformsUbo(MaxNumBatchedColors),
        m_ColorsUbo(MaxNumBatchedColors),
        m_Projection(1.0f)
    {
        glm::vec3 vertices[4] = {
            glm::vec3{0.0f, 0.0f, 0.2f},
            glm::vec3{1.0f, 0.0f, 0.2f},
            glm::vec3{1.0f, 1.0f, 0.2f},
            glm::vec3{0.0f, 1.0f, 0.2f}
        };

        struct VerticesTrait
        {
            static std::array<EVertexType, 1> GetDataFormat()
            {
                return {EVertexType::Vec3};
            }
        };

        GLuint indices[] =
        {
            0, 1, 2,
            2, 3, 0
        };

        m_VertexArray.AddBuffer(TVertexBuffer<glm::vec3, VerticesTrait>{vertices});
        m_VertexArray.SetIndexBuffer(IndexBuffer::Create(indices));

        ShaderSource source;
        source.VertexShaderSource = s_VertexShader;
        source.FragmentShaderSource = s_FragmentShader;

        if (!Shader::Create("default", source, m_Shader))
        {
            std::exit(EXIT_FAILURE);
        }

        m_Shader->Bind();
        m_Shader->BindUniformBuffer("Transforms", m_TransformsUbo.GetUniformBuffer());
        m_Shader->BindUniformBuffer("Colors", m_ColorsUbo.GetUniformBuffer());
        m_Shader->SetMat4("u_Projection", m_Projection);
    }

    void AddRectInstance(glm::vec3 pos, glm::vec3 size, glm::vec4 color)
    {
        if (m_Transforms.size() == 400)
        {
            FlushDraw();
        }

        glm::mat4 transform = glm::translate(glm::mat4{1.0f}, pos) * glm::scale(glm::mat4{1.0f}, size);
        m_Transforms.emplace_back(transform);
        m_Colors.emplace_back(color);
    }

    void FlushDraw()
    {
        m_VertexArray.Bind();
        m_Shader->Bind();

        m_Shader->SetMat4("u_Projection", m_Projection);

        m_TransformsUbo.UpdateBuffer(m_Transforms);
        m_Shader->BindUniformBuffer("Transforms", m_TransformsUbo.GetUniformBuffer());

        m_ColorsUbo.UpdateBuffer(m_Colors);
        m_Shader->BindUniformBuffer("Colors", m_ColorsUbo.GetUniformBuffer());

        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, (int)m_Colors.size());

        m_Transforms.clear();
        m_Colors.clear();
    }

    void UpdateProjection(const glm::mat4& projection)
    {
        m_Projection = projection;
    }

private:
    std::vector<glm::mat4> m_Transforms;
    std::vector<glm::vec4> m_Colors;

    TUniformBuffer<glm::mat4> m_TransformsUbo;
    TUniformBuffer<glm::vec4> m_ColorsUbo;
    std::shared_ptr<Shader> m_Shader;
    glm::mat4 m_Projection;

    VertexArray m_VertexArray;
};

RectRenderer::RectRenderer():
    m_Implementation(new RectRendererImpl())
{
}

RectRenderer::~RectRenderer() noexcept
{
    delete m_Implementation;
}

void RectRenderer::AddRectInstance(glm::vec3 pos, glm::vec3 size, glm::vec4 color)
{
    m_Implementation->AddRectInstance(pos, size, color);
}

void RectRenderer::FlushDraw()
{
    m_Implementation->FlushDraw();
}

void RectRenderer::UpdateProjection(glm::mat4 projection)
{
    m_Implementation->UpdateProjection(projection);
}