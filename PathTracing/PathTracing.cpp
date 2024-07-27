// PathTracing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

#include <unordered_map>

#include "Shader.h"
#include "Buffers.h"
#include "VertexArray.h"
#include "UniformBuffer.h"

#include "RectRenderer.h"
#include "LineBatch.h"
#include "Map.h"
#include "AStarAlgorithm.h"

static GLFWwindow* s_Window = nullptr;

static void ExitGame()
{
    glfwDestroyWindow(s_Window);
    s_Window = nullptr;
    glfwTerminate();
}

const int MapWidth = 20;
const int MapHeight = 20;

static const float InitialWindowSizeX = 1280.0f;
static const float InitialWindowSizeY = 720.0f;

static glm::mat4 s_Projection = glm::ortho(0.0f, InitialWindowSizeX, 0.0f, InitialWindowSizeY, -10.0f, 10.0f);
static float s_WindowWidth, s_WindowHeight;
static float s_CellSize = InitialWindowSizeX / MapWidth;

static glm::vec4 GetColorForField(EFieldType field)
{
    switch (field)
    {
    case EFieldType::Empty:
        return glm::vec4(0.25f);
    case EFieldType::Obstacle:
        return glm::vec4{0.5f, 0.2f, 0.2f, 1.0f};
    default:
        break;
    }
    return glm::vec4{0.0f};
}

static float GetRandomFloat()
{
    struct SrandInitializer
    {
        SrandInitializer()
        {
            std::srand(std::time(nullptr));
        }
    };

    static SrandInitializer srandInitializer;
    return (float)std::rand() / (float)RAND_MAX;
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    Map map(MapWidth, MapHeight);

    std::atexit(&ExitGame);
    s_Window = glfwCreateWindow(InitialWindowSizeX, InitialWindowSizeY, "Game", nullptr, nullptr);

    {
        int x, y;
        glfwGetWindowSize(s_Window, &x, &y);
        s_WindowWidth = x;
        s_WindowHeight = y;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    if (!s_Window)
    {
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(s_Window);
    glfwSwapInterval(1);

    if (!gladLoadGL())
    {
        return EXIT_FAILURE;
    }

    RectRenderer rectRenderer;
    LineBatch lineBatch;

    lineBatch.Thickness = 2.0f;

    for (int i = 0; i < 10; ++i)
    {
        glm::ivec2 pos(std::rand() % MapWidth, std::rand() % MapHeight);

        map.SetField(pos.x, pos.y, EFieldType::Obstacle);
    }

    AStarAlgorithm algorithm;

    Path path = std::move(algorithm.FindPathTo(PathFindingPoint(0, 0), PathFindingPoint(10, 5), &map));

    std::cout << "Path found:\n";
    for (const auto& p : path)
    {
        std::cout << "(" << p.x << ", " << p.y << ")\n";
    }

    while (!glfwWindowShouldClose(s_Window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rectRenderer.UpdateProjection(s_Projection);
        lineBatch.OnBeginScene(s_Projection);

        for (auto [pos, field] : map)
        {
            float posY = pos.y;
            float posX = pos.x;

            posY *= s_CellSize;
            posX *= s_CellSize;

            glm::vec4 color = GetColorForField(field);
            color *= 0.4f;

            /* Render bounds first */
            rectRenderer.AddRectInstance(glm::vec3{posX, posY, -1.0f},
                glm::vec3{s_CellSize, s_CellSize, 0.0f}, color);

            /* Now render right field */
            rectRenderer.AddRectInstance(glm::vec3{posX + 2.5, posY + 2.5, -1.0f},
                glm::vec3{s_CellSize - 5, s_CellSize - 5, 0.0f},
                GetColorForField(field));
        }

        for (size_t i = 0; path.size() > 0 && i < path.size() - 1; ++i)
        {
            glm::vec3 pos = glm::vec3{path[i], 0};
            glm::vec3 nextpos = glm::vec3{path[i + 1], 0};

            for (int j = 0; j < pos.length(); ++j)
            {
                pos[j] *= s_CellSize;
                nextpos[j] *= s_CellSize;
            }

            pos.x -= s_CellSize / 2;
            pos.y += s_CellSize / 2;
            nextpos.x -= s_CellSize / 2;
            nextpos.y += s_CellSize / 2;

            lineBatch.DrawLine(pos, nextpos, glm::mat4{1.0f});
        }

        rectRenderer.FlushDraw();
        lineBatch.FlushDraw();
        glfwSwapBuffers(s_Window);
    }

    return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

