#pragma once

#include <glm/glm.hpp>

struct DrawCommandArgs
{
    glm::vec4 Color{1.0f};
    glm::mat4 Transform{1.0f};
};

class Renderer
{
public:
    static void Initialize();
    static void Quit();

public:
    static void DrawLine(glm::vec3 start, glm::vec3 end, const DrawCommandArgs& args = DrawCommandArgs{});
    static void DrawRect(glm::vec3 position, glm::vec3 size, const DrawCommandArgs& args = DrawCommandArgs{});
    static void BeginScene(const glm::mat4& projection);
    static void EndScene();
    static void FlushDraw();
};

