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
static bool s_bClickedMouseLastFrame = false;

static glm::mat4 s_Projection = glm::ortho(0.0f, InitialWindowSizeX, 0.0f, InitialWindowSizeY, -10.0f, 10.0f);
static float s_WindowWidth, s_WindowHeight;

static float SnapToGrid(float value, float gridSize)
{
    return gridSize * (int)(value / gridSize);
}

struct Player
{
    glm::ivec2 Pos{0,0};
    glm::ivec2 PrevPos{0, 0};

    Path CurrentPath;
    size_t CurrentNodeIndex = 0;

    glm::vec2 interpolatedPos = Pos;
    PathFindingPoint Goal;

    void Move(IPathFindingAlgorithm* algorithm, Map* map)
    {
        if (CurrentNodeIndex >= CurrentPath.size())
        {
            return;
        }

        PrevPos = Pos;
        Pos = CurrentPath[CurrentNodeIndex];

        EFieldType field = map->GetFieldAt(Pos.x, Pos.y);
        if ((field != EFieldType::Empty && field != EFieldType::Goal) && Pos != PrevPos)
        {
            glm::ivec2 goal = CurrentPath.back();
            Pos = PrevPos;
            RecalculatePath(algorithm, map);
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

            if (IsAlreadyOccupiedBySomeone(map, point))
            {
                point = Pos;
            }

            interpolatedPos = glm::mix(interpolatedPos, glm::vec2(point), 0.125f);

            if (glm::distance(interpolatedPos, glm::vec2(Pos)) < 0.1f)
            {
                interpolatedPos = Pos;
            }
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

    bool IsAlreadyOccupiedBySomeone(const Map* map, PathFindingPoint point) const
    {
        return point != Pos && map->GetFieldAt(Pos.x, Pos.y) != EFieldType::Empty && map->GetFieldAt(Pos.x, Pos.y) != EFieldType::Goal;
    }

    void RecalculatePath(IPathFindingAlgorithm* algorithm, Map* map)
    {
        if (!CurrentPath.empty())
        {
            Goal = CurrentPath.back();
        }
        CurrentPath = std::move(algorithm->FindPathTo(Pos, Goal, map));
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

    glfwSetMouseButtonCallback(s_Window, [](GLFWwindow* window, int key, int action, int mods)
    {
        if (action == GLFW_PRESS && key == GLFW_MOUSE_BUTTON_LEFT)
        {
            s_bClickedMouseLastFrame = true;
        }
    });

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

    bool bShowTestWindow = false;

    const char* pathFindingModes[] = {
        "A* Path finding",
        "Genetic path finding (Not deterministics, may be not sufficient to implement this in game)"
    };

    const char* modes[] = {
        "Selecting target",
        "Placing obstacles",
        "Remove agent",
        "Place agent"
    };

    typedef std::chrono::system_clock SystemClock;

    constexpr size_t MaxAgents = 10;

    auto startTime = SystemClock::now();

    std::vector<Player> players;
    players.emplace_back();
    players.back().Pos = start;
    players.back().Goal = start;
    int targetPlayer = 0;

    const char* agents[MaxAgents] = {
        "Agent 0", "Agent 1", "Agent 2", "Agent 3", "Agent 4", "Agent 5", "Agent 6", "Agent 7", "Agent 8", "Agent 9"
    };

    int numRightClickOptions = IM_ARRAYSIZE(modes);

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

            static int rightClickOperationIndex = 1;
            static bool bAutoSwitchToSelectingDestination = true;

            if (ImGui::Combo("Right click operation", &rightClickOperationIndex, modes, numRightClickOptions))
            {
            }

            if (s_bClickedMouseLastFrame && !(ImGui::IsWindowHovered() || ImGui::IsWindowFocused()))
            {
                double x, y;
                glfwGetCursorPos(s_Window, &x, &y);

                y = s_WindowHeight - y;
                x /= map.CellSize;
                y /= map.CellSize;

                x = SnapToGrid(x, 1);
                y = SnapToGrid(y, 1);

                if (rightClickOperationIndex == 0)
                {
                    if (!players.empty())
                    {
                        glm::ivec2 copy = players[targetPlayer].Goal;

                        map.SetField(players[targetPlayer].Goal.x, players[targetPlayer].Goal.y, EFieldType::Empty);
                        players[targetPlayer].Goal = {(int)x, (int)y};

                        if (map.GetFieldAt(players[targetPlayer].Goal.x, players[targetPlayer].Goal.y) == EFieldType::Empty)
                        {
                            players[targetPlayer].CurrentPath = pathFindingAlgorithm->FindPathTo(players[targetPlayer].Pos,
                                players[targetPlayer].Goal, &map);
                            players[targetPlayer].CurrentNodeIndex = 0;
                        }
                        else
                        {
                            players[targetPlayer].Goal = copy;
                        }

                        map.SetField(players[targetPlayer].Goal.x, players[targetPlayer].Goal.y, EFieldType::Goal);
                    }
                }
                else if (rightClickOperationIndex == 1)
                {
                    EFieldType field = map.GetFieldAt((int)x, (int)y);

                    if (field == EFieldType::Obstacle)
                    {
                        map.SetField((int)x, (int)y, EFieldType::Empty);
                    }
                    else if (field == EFieldType::Empty)
                    {
                        map.SetField((int)x, (int)y, EFieldType::Obstacle);
                    }
                }
                else if (rightClickOperationIndex == 2)
                {
                    PathFindingPoint placePoint = {(int)x, (int)y};

                    if (map.GetFieldAt(placePoint.x, placePoint.y) == EFieldType::Player)
                    {
                        auto i = std::find_if(players.begin(), players.end(), [&](const Player& player)
                        {
                            return player.Pos == placePoint;
                        });

                        if (i != players.end())
                        {
                            if (targetPlayer >= players.size() - 1)
                            {
                                targetPlayer = 0;
                            }

                            map.SetField(placePoint.x, placePoint.y, EFieldType::Empty);
                            players.erase(i);

                            if (players.empty())
                            {
                                numRightClickOptions = IM_ARRAYSIZE(modes);
                            }

                        }

                        if (bAutoSwitchToSelectingDestination)
                        {
                            rightClickOperationIndex = 0;
                        }
                    }

                    if (players.size() == MaxAgents)
                    {
                        numRightClickOptions = IM_ARRAYSIZE(modes) - 1;
                    }
                }
                else if (rightClickOperationIndex == 3)
                {
                    PathFindingPoint placePoint = {(int)x, (int)y};

                    if (players.size() < MaxAgents && map.GetFieldAt(placePoint.x, placePoint.y) == EFieldType::Empty)
                    {
                        targetPlayer = (int)players.size();
                        players.emplace_back();
                        players[targetPlayer].Pos = players[targetPlayer].PrevPos = placePoint;

                        players[targetPlayer].CurrentNodeIndex = 0;

                        if (bAutoSwitchToSelectingDestination)
                        {
                            rightClickOperationIndex = 0;
                        }
                    }

                    if (players.size() == MaxAgents)
                    {
                        rightClickOperationIndex = 0;
                        numRightClickOptions = IM_ARRAYSIZE(modes) - 1;
                    }
                }
            }

            ImGui::Checkbox("bAutoSwitchToTargetPostAddedAgent", &bAutoSwitchToSelectingDestination);

            if (&geneticPathFinding == pathFindingAlgorithm)
            {
                static float MutRate = 0.01f;

                ImGui::LabelText("Genetic algorithm settings", "Genetic algorithm settings");
                if (ImGui::DragFloat("MutationRate", &MutRate, 0.01f, 0.01f, 1.0f))
                {
                    geneticPathFinding.MutationRate = MutRate;

                    for (Player& player : players)
                    {
                        player.RecalculatePath(pathFindingAlgorithm, &map);
                    }
                }

                static int NumGens = 100;

                if (ImGui::DragInt("NumGenerations", &NumGens, 1, 10, 1000))
                {
                    geneticPathFinding.NumGenerations = NumGens;

                    for (Player& player : players)
                    {
                        player.RecalculatePath(pathFindingAlgorithm, &map);
                    }
                }

                static int PopSize = 100;

                if (ImGui::DragInt("Population size", &PopSize, 1, 10, 1000))
                {
                    geneticPathFinding.PopulationSize = PopSize;

                    for (Player& player : players)
                    {
                        player.RecalculatePath(pathFindingAlgorithm, &map);
                    }
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

        s_bClickedMouseLastFrame = false;
        glfwSwapBuffers(s_Window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return EXIT_SUCCESS;
}