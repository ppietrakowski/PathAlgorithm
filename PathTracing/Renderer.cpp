#include "Renderer.h"
#include "LineBatch.h"
#include "RectRenderer.h"

struct RendererData
{
    LineBatch lineBatch;
    RectRenderer rectRenderer;
    glm::mat4 projection{1.0f};

    RendererData()
    {
        lineBatch.Thickness = 2.0f;
    }

    void FlushDraw()
    {
        lineBatch.FlushDraw();
        rectRenderer.FlushDraw();
    }
};

static RendererData* s_RendererData = nullptr;

void Renderer::Initialize()
{
    s_RendererData = new RendererData();
}

void Renderer::Quit()
{
    delete s_RendererData;
    s_RendererData = nullptr;
}

void Renderer::DrawLine(glm::vec3 start, glm::vec3 end, const DrawCommandArgs& args)
{
    s_RendererData->lineBatch.DrawLine(start, end, args.Transform, args.Color);
}

void Renderer::DrawRect(glm::vec3 position, glm::vec3 size, const DrawCommandArgs& args)
{
    s_RendererData->rectRenderer.AddRectInstance(position, size, args.Color);
}

void Renderer::BeginScene(const glm::mat4& projection)
{
    s_RendererData->projection = projection;
    s_RendererData->lineBatch.OnBeginScene(projection);
    s_RendererData->rectRenderer.UpdateProjection(projection);
}

void Renderer::EndScene()
{
    s_RendererData->FlushDraw();
}

void Renderer::FlushDraw()
{
    s_RendererData->FlushDraw();
}
