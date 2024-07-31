#pragma once

#include "PathFindingAlgorithm.h"
#include "Map.h"

class Player
{
public:
    Player() = default;
    Player(PathFindingPoint startPos, PathFindingPoint goalPos, glm::vec4 lineColor = glm::vec4{1.0f});

    void Move(IPathFindingAlgorithm* algorithm, Map* map);
    void Draw(Map& map);

    bool IsAlreadyOccupiedBySomeone(const Map& map, PathFindingPoint point) const;
    void RecalculatePath(IPathFindingAlgorithm* algorithm, Map* map);

    void SetNewGoal(PathFindingPoint newGoal, Map& map, IPathFindingAlgorithm *pathFindingAlgorithm);

    PathFindingPoint GetGridPosition() const;

    void DrawImGuiLineColorSelection();

private:
    glm::ivec2 m_Position{0,0};
    glm::ivec2 m_PrevPosition{0, 0};

    Path m_CurrentPath;
    size_t m_CurrentNodeIndex = 0;

    glm::vec2 m_InterpolatedPos = m_Position;
    PathFindingPoint m_Goal;
    glm::vec4 m_LineColor{1.0f};

private:
    void DrawPath(glm::vec3 start, glm::vec3 end, Map& map);
    void InterpolateMovement(const Map& map);
};

