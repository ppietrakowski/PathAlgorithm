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

enum class EFieldType : int16_t
{
    Empty = 0,
    Obstacle
};

class MapIterator
{
public:
    MapIterator(const class Map* map, glm::ivec2 pos);

    bool operator==(const MapIterator& i) const
    {
        return i.m_Pos == m_Pos;
    }

    bool operator!=(const MapIterator& i) const
    {
        return i.m_Pos != m_Pos;
    }

    MapIterator operator++(int) const;
    MapIterator& operator++();

    std::pair<glm::ivec2, EFieldType> operator*() const;

private:
    const class Map* m_Map;
    glm::ivec2 m_Pos;
};

class Map
{
public:
    Map(int32_t width, int32_t height);

    EFieldType GetField(int32_t x, int32_t y) const;
    void SetField(int32_t x, int32_t y, EFieldType field);

    bool IsEmpty(int32_t x, int32_t y) const;

    int32_t GetMapWidth() const;
    int32_t GetMapHeight() const;

    MapIterator begin() const
    {
        return MapIterator{this, glm::ivec2(0, 0)};
    }

    MapIterator end() const
    {
        return MapIterator{this, glm::ivec2(0, m_Height)};
    }

private:
    std::vector<EFieldType> m_Fields;
    int32_t m_Width;
    int32_t m_Height;
};

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
    
    for (int i = 0; i < 10; ++i)
    {
        glm::ivec2 pos(std::rand() % MapWidth, std::rand() % MapHeight);

        map.SetField(pos.x, pos.y, EFieldType::Obstacle);
    }

    while (!glfwWindowShouldClose(s_Window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        rectRenderer.UpdateProjection(s_Projection);

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

Map::Map(int32_t width, int32_t height):
    m_Fields(static_cast<size_t>(width * height), EFieldType::Empty),
    m_Width(width),
    m_Height(height)
{
}

EFieldType Map::GetField(int32_t x, int32_t y) const
{
    return m_Fields[x + y * m_Width];
}

void Map::SetField(int32_t x, int32_t y, EFieldType field)
{
    m_Fields[x + y * m_Width] = field;
}

bool Map::IsEmpty(int32_t x, int32_t y) const
{
    return GetField(x, y) == EFieldType::Empty;
}

int32_t Map::GetMapWidth() const
{
    return m_Width;
}

int32_t Map::GetMapHeight() const
{
    return m_Height;
}

MapIterator::MapIterator(const Map* map, glm::ivec2 pos):
    m_Map(map),
    m_Pos(pos)
{
}

MapIterator MapIterator::operator++(int) const
{
    glm::ivec2 pos = m_Pos;
    pos.x++;

    if (pos.x >= m_Map->GetMapWidth())
    {
        pos.x = 0;
        pos.y++;
    }

    return MapIterator{m_Map, pos};
}

MapIterator& MapIterator::operator++()
{
    m_Pos.x++;

    if (m_Pos.x >= m_Map->GetMapWidth())
    {
        m_Pos.x = 0;
        m_Pos.y++;
    }

    return *this;
}

std::pair<glm::ivec2, EFieldType> MapIterator::operator*() const
{
    return {m_Pos, m_Map->GetField(m_Pos.x, m_Pos.y)};
}
