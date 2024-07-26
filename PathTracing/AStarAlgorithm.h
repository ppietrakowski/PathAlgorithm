#pragma once

#include "PathFindingAlgorithm.h"

class AStarAlgorithm : public IPathFindingAlgorithm
{
public:
    virtual Path FindPathTo(PathFindingPoint start, PathFindingPoint goal, const IMap* map) override;
};

