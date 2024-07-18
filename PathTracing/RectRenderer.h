#pragma once

#include <glm/glm.hpp>

class RectRenderer
{
    class RectRendererImpl;
public:
    RectRenderer();
    ~RectRenderer() noexcept;

    void AddRectInstance(glm::vec3 pos, glm::vec3 size, glm::vec4 color);
    void FlushDraw();

    void UpdateProjection(glm::mat4 projection);

private:
    RectRendererImpl* m_Implementation = nullptr;
};

