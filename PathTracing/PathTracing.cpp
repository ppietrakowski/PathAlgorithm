// PathTracing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

#include <unordered_map>

#include "Shader.h"
#include "Buffers.h"
#include "VertexArray.h"
#include "UniformBuffer.h"

#include "RectRenderer.h"

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

static int s_Fields[MapWidth * MapHeight];
static glm::vec4 s_ColorMap[MapWidth * MapHeight];
static glm::mat4 s_Projection = glm::ortho(0.0f, InitialWindowSizeX, 0.0f, InitialWindowSizeY, -10.0f, 10.0f);
static int s_WindowWidth, s_WindowHeight;
static float s_CellSize = InitialWindowSizeX / MapWidth;

static const int FieldEmpty = 0;

static glm::vec4 GetColorForField(int field)
{
    switch (field)
    {
    case FieldEmpty:
        return glm::vec4(0.25f);
    default:
        break;
    }
    return glm::vec4{0.0f};
}

int main()
{
    if (!glfwInit())
    {
        return EXIT_FAILURE;
    }

    std::atexit(&ExitGame);
    s_Window = glfwCreateWindow(InitialWindowSizeX, InitialWindowSizeY, "Game", nullptr, nullptr);

    glfwGetWindowSize(s_Window, &s_WindowWidth, &s_WindowHeight);

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
    
    srand(time(0));

    auto randomFloat = []() -> float
    {
        return (float)rand() / (float)RAND_MAX;
    };

    for (int i = 0; i < std::ssize(s_Fields); ++i)
    {
        s_Fields[i] = 0;
        glm::vec4 color{0.0f};
        for (int k = 0; k < color.length(); ++k)
        {
            color[k] = randomFloat();
        }
        
        s_ColorMap[i] = color;
    }

    while (!glfwWindowShouldClose(s_Window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rectRenderer.UpdateProjection(s_Projection);

        /* Render bounds first */
        for (int i = 0; i < std::ssize(s_Fields); ++i)
        {
            float posY = i / MapWidth;
            float posX = i % MapWidth;

            posY *= s_CellSize;
            posX *= s_CellSize;

            glm::vec4 color = GetColorForField(s_Fields[i]);
            color *= 0.4f;

            rectRenderer.AddRectInstance(glm::vec3{posX, posY, -1.0f},
                glm::vec3{s_CellSize, s_CellSize, 0.0f}, color);

            /* Now render right field */
            rectRenderer.AddRectInstance(glm::vec3{posX + 2.5, posY + 2.5, -1.0f},
                glm::vec3{s_CellSize - 5, s_CellSize - 5, 0.0f},
                GetColorForField(s_Fields[i]));
        }

        rectRenderer.FlushDraw();
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
