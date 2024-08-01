#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <chrono>

#include "Map.h"
#include "Player.h"

typedef std::chrono::system_clock SystemClock;
constexpr size_t MaxAgents = 10;

class Application
{
public:
    Application(int width, int height, const char* title);
    ~Application() noexcept;

    void Run();

    static inline const int MapWidth = 30;
    static inline const int MapHeight = 10;
    static inline const float CellSize = 64;

private:
    GLFWwindow* m_Window;
    float m_Width;
    float m_Height;
    glm::mat4 m_Projection;

    bool m_bClickedMouseLastFrame = false;
    std::shared_ptr<Map> m_Map;

    const char* m_PathFindingModes[2] = {
        "A* Path finding",
        "Genetic path finding (Not deterministics, may be not sufficient to implement this in game)"
    };

    const char* m_Modes[4] = {
        "Selecting target",
        "Placing obstacles",
        "Remove agent",
        "Place agent"
    };

    std::vector<Player> m_Players;
    int m_TargetPlayer;

    const char* m_AgentsName[MaxAgents] = {
        "Agent 0", "Agent 1", "Agent 2", "Agent 3", "Agent 4", "Agent 5", "Agent 6", "Agent 7", "Agent 8", "Agent 9"
    };

    int m_NumRightClickOptions = 4;

    SystemClock::time_point m_StartTime;

private:
    static void MouseKeyCallback(GLFWwindow* window, int key, int action, int mods);
    bool IsAiUpdateFrame() const;

    void AiUpdate();
};

