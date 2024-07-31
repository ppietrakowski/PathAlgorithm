#pragma once

#include "PathFindingAlgorithm.h"

class AStarAlgorithm : public IPathFindingAlgorithm
{
public:
    AStarAlgorithm();
    ~AStarAlgorithm() noexcept;

    virtual Path FindPathTo(PathFindingPoint start, PathFindingPoint goal, const IMap* map) override;

private:
    struct Node* m_Buffer;
    struct Node* m_CurrentAllocationNode;

private:
    struct Node* AllocateNode(PathFindingPoint p, struct Node* parent);
};

