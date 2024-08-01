#include "Player.h"
#include "Renderer.h"
#include "imgui/imgui.h"

Player::Player(PathFindingPoint startPos, PathFindingPoint goalPos, glm::vec4 lineColor) :
    m_Position(startPos),
    m_Goal(goalPos),
    m_LineColor(lineColor)
{
}

void Player::Move()
{
    if (m_CurrentNodeIndex >= m_CurrentPath.size())
    {
        return;
    }

    m_PrevPosition = m_Position;
    m_Position = m_CurrentPath[m_CurrentNodeIndex];

    auto map = IMap::GetInstance();
    EFieldType field = map->GetFieldAt(m_Position);
    if ((field != EFieldType::Empty && field != EFieldType::Goal) && m_Position != m_PrevPosition)
    {
        m_Position = m_PrevPosition;
        RecalculatePath();
        m_CurrentNodeIndex = 0;
        return;
    }

    ++m_CurrentNodeIndex;
}

void Player::Draw()
{
    auto map = IMap::GetInstance();

    map->SetField(m_PrevPosition, EFieldType::Empty);
    map->SetField(m_Position, EFieldType::Player);

    if (m_InterpolatedPos == glm::vec2(0.0f))
    {
        m_InterpolatedPos = m_Position;
    }
    else
    {
        InterpolateMovement();
    }

    float cellSize = map->GetCellSize();
    float posX = m_InterpolatedPos.x;
    float posY = m_InterpolatedPos.y;

    posY *= cellSize;
    posX *= cellSize;

    /* Draw little purple rect to indicate player position */
    Renderer::DrawRect(glm::vec3{posX + 12.5, posY + 12.5, -1.0f},
        glm::vec3{cellSize - 25, cellSize - 25, 0.0f},
        DrawCommandArgs{GetColorForField(EFieldType::Player)});

    /* Quit quickly, because otherwise index would overflow */
    if (m_CurrentPath.empty())
    {
        return;
    }

    /* Draw line from player middle to next node in path */
    if (m_CurrentNodeIndex + 1 < m_CurrentPath.size())
    {
        glm::vec3 nextpos = glm::vec3{m_CurrentPath[m_CurrentNodeIndex + 1], 0};
        glm::vec3 pos{m_InterpolatedPos, 0};
        DrawPath(pos, nextpos);
    }

    for (size_t i = m_CurrentNodeIndex + 1; i < m_CurrentPath.size() - 1; ++i)
    {
        glm::vec3 pos = glm::vec3{m_CurrentPath[i], 0};
        glm::vec3 nextpos = glm::vec3{m_CurrentPath[i + 1], 0};
        DrawPath(pos, nextpos);
    }
}

bool Player::IsAlreadyOccupiedBySomeone(PathFindingPoint point) const
{
    auto map = IMap::GetInstance();

    return point != m_Position && map->GetFieldAt(m_Position) != EFieldType::Empty && map->GetFieldAt(m_Position) != EFieldType::Goal;
}

void Player::RecalculatePath()
{
    auto map = IMap::GetInstance();

    if (!m_CurrentPath.empty())
    {
        m_Goal = m_CurrentPath.back();
    }

    m_CurrentPath = std::move(PathFindingAlgorithm::FindPathTo(m_Position, m_Goal));
}

void Player::SetNewGoal(PathFindingPoint newGoal)
{
    auto map = IMap::GetInstance();

    glm::ivec2 oldGoal = m_Goal;

    map->SetField(oldGoal, EFieldType::Empty);
    m_Goal = newGoal;

    if (map->GetFieldAt(m_Goal) == EFieldType::Empty)
    {
        m_CurrentPath = PathFindingAlgorithm::FindPathTo(m_Position, m_Goal);
        m_CurrentNodeIndex = 0;
    }
    else
    {
        m_Goal = oldGoal;
    }

    map->SetField(m_Goal, EFieldType::Goal);
}

PathFindingPoint Player::GetGridPosition() const
{
    return m_Position;
}

void Player::DrawImGuiLineColorSelection()
{
    ImGui::ColorEdit4("Agent line color: ", &m_LineColor[0]);
}

void Player::DrawPath(glm::vec3 start, glm::vec3 end)
{
    auto map = IMap::GetInstance();

    float cellSize = map->GetCellSize();

    for (int32_t j = 0; j < start.length(); ++j)
    {
        start[j] *= cellSize;
        end[j] *= cellSize;
    }

    start.x += cellSize / 2;
    start.y += cellSize / 2;
    end.x += cellSize / 2;
    end.y += cellSize / 2;

    start.z = 1.5f;
    end.z = 1.5f;

    Renderer::DrawLine(start, end, DrawCommandArgs{m_LineColor});
}

void Player::InterpolateMovement()
{
    PathFindingPoint point = m_Position;
    if (m_CurrentNodeIndex < m_CurrentPath.size())
    {
        point = m_CurrentPath[m_CurrentNodeIndex];
    }

    if (IsAlreadyOccupiedBySomeone(point))
    {
        point = m_Position;
    }

    m_InterpolatedPos = glm::mix(m_InterpolatedPos, glm::vec2(point), 0.125f);

    if (glm::distance(m_InterpolatedPos, glm::vec2(m_Position)) < 0.1f)
    {
        m_InterpolatedPos = m_Position;
    }
}
