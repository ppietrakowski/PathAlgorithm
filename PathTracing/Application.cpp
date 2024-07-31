#include "Application.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <chrono>

typedef std::chrono::system_clock SystemClock;

static float SnapToGrid(float value, float gridSize)
{
    return gridSize * (int)(value / gridSize);
}

static Application* s_AppInstance = nullptr;

Application::Application(int width, int height, const char* title) :
    m_Width(width),
    m_Height(height),
    m_Projection(glm::ortho(0.0f, m_Width, 0.0f, m_Height, -10.0f, 10.0f)),
    m_Map(MapWidth, MapHeight)
{
    if (!glfwInit())
    {
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_RESIZABLE, false);

    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    if (!gladLoadGL())
    {
        std::exit(EXIT_FAILURE);
    }

    glfwSetMouseButtonCallback(m_Window, &Application::MouseKeyCallback);
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

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    for (int i = 0; i < 10; ++i)
    {
        glm::ivec2 pos(std::rand() % MapWidth, std::rand() % MapHeight);
        m_Map.SetField(pos.x, pos.y, EFieldType::Obstacle);
    }

    m_PathFindingAlgorithm = &m_AStarAlgorithm;
    Renderer::Initialize();
    s_AppInstance = this;
}

Application::~Application() noexcept
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    Renderer::Quit();
    s_AppInstance = nullptr;
}

void Application::Run()
{
    auto startTime = SystemClock::now();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (SystemClock::now() - startTime >= std::chrono::milliseconds{300})
        {
            startTime = SystemClock::now();

            for (Player& player : m_Players)
            {
                player.Move(m_PathFindingAlgorithm, &m_Map);
            }
        }

        Renderer::BeginScene(m_Projection);
        m_Map.Draw(m_Projection);
        for (Player& player : m_Players)
        {
            player.Draw(m_Map);
        }

        Renderer::EndScene();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        static int selectedPathAlgo;

        ImGui::Begin("Settings");
        if (ImGui::Combo("Path finding algorithm", &selectedPathAlgo, m_PathFindingModes, IM_ARRAYSIZE(m_PathFindingModes)))
        {
            IPathFindingAlgorithm* newPathAlgorithm = nullptr;

            switch (selectedPathAlgo)
            {
            case 0:
                newPathAlgorithm = &m_AStarAlgorithm;
                break;
            case 1:
                newPathAlgorithm = &m_GeneticPathFinding;
                break;
            default:
                break;
            }

            if (newPathAlgorithm != m_PathFindingAlgorithm)
            {
                for (Player& player : m_Players)
                {
                    player.RecalculatePath(newPathAlgorithm, &m_Map);
                }

                m_PathFindingAlgorithm = newPathAlgorithm;
            }
        }

        static int rightClickOperationIndex = 1;
        static bool bAutoSwitchToSelectingDestination = true;

        if (ImGui::Combo("Right click operation", &rightClickOperationIndex, m_Modes, m_NumRightClickOptions))
        {
        }

        if (m_bClickedMouseLastFrame && !(ImGui::IsWindowHovered() || ImGui::IsWindowFocused()))
        {
            double x, y;
            glfwGetCursorPos(m_Window, &x, &y);

            y = m_Height - y;
            x /= m_Map.CellSize;
            y /= m_Map.CellSize;

            x = SnapToGrid(x, 1);
            y = SnapToGrid(y, 1);

            if (rightClickOperationIndex == 0)
            {
                if (!m_Players.empty())
                {
                    m_Players[m_TargetPlayer].SetNewGoal({(int)x, (int)y}, m_Map, m_PathFindingAlgorithm);
                }
            }
            else if (rightClickOperationIndex == 1)
            {
                EFieldType field = m_Map.GetFieldAt((int)x, (int)y);

                if (field == EFieldType::Obstacle)
                {
                    m_Map.SetField((int)x, (int)y, EFieldType::Empty);
                }
                else if (field == EFieldType::Empty)
                {
                    m_Map.SetField((int)x, (int)y, EFieldType::Obstacle);
                }
            }
            else if (rightClickOperationIndex == 2)
            {
                PathFindingPoint placePoint = {(int)x, (int)y};

                if (m_Map.GetFieldAt(placePoint.x, placePoint.y) == EFieldType::Player)
                {
                    auto i = std::find_if(m_Players.begin(), m_Players.end(), [&](const Player& player)
                    {
                        return player.GetGridPosition() == placePoint;
                    });

                    if (i != m_Players.end())
                    {
                        if (m_TargetPlayer >= m_Players.size() - 1)
                        {
                            m_TargetPlayer = 0;
                        }

                        m_Map.SetField(placePoint.x, placePoint.y, EFieldType::Empty);
                        m_Players.erase(i);

                        if (m_Players.empty())
                        {
                            m_NumRightClickOptions = IM_ARRAYSIZE(m_Modes);
                        }

                    }

                    if (bAutoSwitchToSelectingDestination)
                    {
                        rightClickOperationIndex = 0;
                    }
                }

                if (m_Players.size() == MaxAgents)
                {
                    m_NumRightClickOptions = IM_ARRAYSIZE(m_Modes) - 1;
                }
            }
            else if (rightClickOperationIndex == 3)
            {
                PathFindingPoint placePoint = {(int)x, (int)y};

                if (m_Players.size() < MaxAgents && m_Map.GetFieldAt(placePoint.x, placePoint.y) == EFieldType::Empty)
                {
                    m_TargetPlayer = (int)m_Players.size();
                    m_Players.emplace_back(placePoint, placePoint);

                    if (bAutoSwitchToSelectingDestination)
                    {
                        rightClickOperationIndex = 0;
                    }
                }

                if (m_Players.size() == MaxAgents)
                {
                    rightClickOperationIndex = 0;
                    m_NumRightClickOptions = IM_ARRAYSIZE(m_Modes) - 1;
                }
            }
        }

        ImGui::Checkbox("bAutoSwitchToTargetPostAddedAgent", &bAutoSwitchToSelectingDestination);

        if (&m_GeneticPathFinding == m_PathFindingAlgorithm)
        {
            static float MutRate = 0.01f;

            ImGui::LabelText("Genetic algorithm settings", "Genetic algorithm settings");
            if (ImGui::DragFloat("MutationRate", &MutRate, 0.01f, 0.01f, 1.0f))
            {
                m_GeneticPathFinding.MutationRate = MutRate;

                for (Player& player : m_Players)
                {
                    player.RecalculatePath(m_PathFindingAlgorithm, &m_Map);
                }
            }

            static int NumGens = 100;

            if (ImGui::DragInt("NumGenerations", &NumGens, 1, 10, 1000))
            {
                m_GeneticPathFinding.NumGenerations = NumGens;

                for (Player& player : m_Players)
                {
                    player.RecalculatePath(m_PathFindingAlgorithm, &m_Map);
                }
            }

            static int PopSize = 100;

            if (ImGui::DragInt("Population size", &PopSize, 1, 10, 1000))
            {
                m_GeneticPathFinding.PopulationSize = PopSize;

                for (Player& player : m_Players)
                {
                    player.RecalculatePath(m_PathFindingAlgorithm, &m_Map);
                }
            }
        }

        ImGui::Combo("Agents", &m_TargetPlayer, m_AgentsName, (int)m_Players.size());

        if (!m_Players.empty())
        {
            m_Players[m_TargetPlayer].DrawImGuiLineColorSelection();
        }

        ImGui::End();

        ImGui::Render();
        int32_t display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(m_Window);
        }

        m_bClickedMouseLastFrame = false;
        glfwSwapBuffers(m_Window);
    }
}

void Application::MouseKeyCallback(GLFWwindow* window, int key, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_MOUSE_BUTTON_LEFT)
    {
        s_AppInstance->m_bClickedMouseLastFrame = true;
    }
}
