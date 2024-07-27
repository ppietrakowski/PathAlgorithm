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
#include <chrono>

#include "Shader.h"
#include "Buffers.h"
#include "VertexArray.h"
#include "UniformBuffer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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

struct Player
{
    glm::ivec2 Pos{0,0};
    glm::ivec2 PrevPos{0, 0};

    Path CurrentPath;
    size_t CurrentNodeIndex = 0;

    glm::vec2 interpolatedPos = Pos;

    void Move(IPathFindingAlgorithm* algorithm, Map* map)
    {
        if (CurrentNodeIndex >= CurrentPath.size())
        {
            return;
        }

        PrevPos = Pos;
        Pos = CurrentPath[CurrentNodeIndex];

        if (map->GetFieldAt(Pos.x, Pos.y) != EFieldType::Empty)
        {
            glm::ivec2 goal = CurrentPath.back();
            Pos = PrevPos;
            CurrentPath = std::move(algorithm->FindPathTo(PrevPos, goal, map));
            CurrentNodeIndex = 0;
            return;
        }

        ++CurrentNodeIndex;
    }

    void Draw(Map* map)
    {
        map->RemovePlayer(PrevPos);
        map->AddPlayer(Pos);

        if (interpolatedPos == glm::vec2(0.0f))
        {
            interpolatedPos = Pos;
        }
        else
        {
            PathFindingPoint point = Pos;
            if (CurrentNodeIndex < CurrentPath.size())
            {
                point = CurrentPath[CurrentNodeIndex];
            }

            interpolatedPos = glm::mix(interpolatedPos, glm::vec2(point), 0.125f);
        }

        RectRenderer& rectRenderer = map->GetRectRenderer();
        float CellSize = map->CellSize;

        float posX = interpolatedPos.x;
        float posY = interpolatedPos.y;

        posY *= CellSize;
        posX *= CellSize;

        rectRenderer.AddRectInstance(glm::vec3{posX + 12.5, posY + 12.5, -1.0f},
            glm::vec3{CellSize - 25, CellSize - 25, 0.0f},
            GetColorForField(EFieldType::Player));
    }

    void RecalculatePath(IPathFindingAlgorithm* algorithm, Map* map)
    {
        glm::ivec2 goal = CurrentPath.back();
        CurrentPath = std::move(algorithm->FindPathTo(Pos, goal, map));
    }
};

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    ImGui::StyleColorsClassic();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Map map(MapWidth, MapHeight);

    for (int i = 0; i < 10; ++i)
    {
        glm::ivec2 pos(std::rand() % MapWidth, std::rand() % MapHeight);
        map.SetField(pos.x, pos.y, EFieldType::Obstacle);
    }

    AStarAlgorithm algorithm;
    GeneticPathFinding geneticPathFinding;

    IPathFindingAlgorithm* pathFindingAlgorithm = &algorithm;

    glm::ivec2 start{0, 0};
    glm::ivec2 goal{10, 5};

    bool bShowTestWindow = false;

    const char* pathFindingModes[] = {
        "A* Path finding",
        "Genetic path finding"
    };

    typedef std::chrono::system_clock SystemClock;

    auto startTime = SystemClock::now();

    std::vector<Player> players;
    players.emplace_back();
    players.back().Pos = start;
    players.back().CurrentPath = std::move(pathFindingAlgorithm->FindPathTo(start, goal, &map));
    players.back().CurrentNodeIndex = 1;

    while (!glfwWindowShouldClose(s_Window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (SystemClock::now() - startTime >= std::chrono::milliseconds{300})
        {
            startTime = SystemClock::now();

            for (Player& player : players)
            {
                player.Move(pathFindingAlgorithm, &map);
            }
        }

        map.Draw(s_Projection);
        for (Player& player : players)
        {
            player.Draw(&map);
            map.DrawPath(player.CurrentPath, player.CurrentNodeIndex ? player.CurrentNodeIndex - 1 : 0, player.interpolatedPos);
        }

        map.FlushDraw();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (bShowTestWindow)
        {
            ImGui::ShowDemoWindow();
        }
        else
        {
            static int selectedPathAlgo;

            ImGui::Begin("Settings");
            if (ImGui::Combo("Path finding algorithm", &selectedPathAlgo, pathFindingModes, IM_ARRAYSIZE(pathFindingModes)))
            {
                IPathFindingAlgorithm* newPathAlgorithm = nullptr;

                switch (selectedPathAlgo)
                {
                case 0:
                    newPathAlgorithm = &algorithm;
                    break;
                case 1:
                    newPathAlgorithm = &geneticPathFinding;
                    break;
                default:
                    break;
                }

                if (newPathAlgorithm != pathFindingAlgorithm)
                {
                    for (Player& player : players)
                    {
                        player.RecalculatePath(newPathAlgorithm, &map);
                    }

                    pathFindingAlgorithm = newPathAlgorithm;
                }
            }

            ImGui::End();
        }

        ImGui::Render();
        int32_t display_w, display_h;
        glfwGetFramebufferSize(s_Window, &display_w, &display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(s_Window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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

