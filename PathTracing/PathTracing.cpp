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
#include "GeneticPathFinding.h"

static GLFWwindow* s_Window = nullptr;

static void ExitGame()
{
    glfwDestroyWindow(s_Window);
    s_Window = nullptr;
    glfwTerminate();
}

const int MapWidth = 30;
const int MapHeight = 10;
static float s_CellSize = 64;

static const float InitialWindowSizeX = MapWidth * s_CellSize;
static const float InitialWindowSizeY = MapHeight * s_CellSize;

static glm::mat4 s_Projection = glm::ortho(0.0f, InitialWindowSizeX, 0.0f, InitialWindowSizeY, -10.0f, 10.0f);
static float s_WindowWidth, s_WindowHeight;

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    std::atexit(&ExitGame);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_RESIZABLE, false);

    s_Window = glfwCreateWindow(InitialWindowSizeX, InitialWindowSizeY, "Game", nullptr, nullptr);

    {
        int x, y;
        glfwGetWindowSize(s_Window, &x, &y);
        s_WindowWidth = x;
        s_WindowHeight = y;
    }

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

    Map map(MapWidth, MapHeight);

    for (int i = 0; i < 10; ++i)
    {
        glm::ivec2 pos(std::rand() % MapWidth, std::rand() % MapHeight);
        map.SetField(pos.x, pos.y, EFieldType::Obstacle);
    }

    AStarAlgorithm algorithm;
    GeneticPathFinding geneticPathFinding;

    IPathFindingAlgorithm* pathFindingAlgorithm = &algorithm;

    map.AddPlayer(glm::ivec2(8, 5));
    map.AddPlayer(glm::ivec2(8, 4));

    Path path = std::move(pathFindingAlgorithm->FindPathTo(PathFindingPoint(0, 0), PathFindingPoint(10, 5), &map));

    std::cout << "Path found:\n";
    for (const auto& p : path)
    {
        std::cout << "(" << p.x << ", " << p.y << ")\n";
    }

    while (!glfwWindowShouldClose(s_Window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        map.DrawPath(path, 0);
        map.Draw(s_Projection);
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

