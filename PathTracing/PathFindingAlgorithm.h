#pragma once

#include "MapInterface.h"
#include <vector>
#include <unordered_map>

typedef glm::ivec2 PathFindingPoint;

namespace std
{
    template<>
    struct hash<PathFindingPoint>
    {
        size_t operator()(const PathFindingPoint& p) const
        {
            return hash<int32_t>()(p.x) ^ hash<int32_t>()(p.y);
        }
    };
}

typedef std::vector<PathFindingPoint> Path;

class IPathFindingAlgorithm
{
public:
    virtual ~IPathFindingAlgorithm() = default;

    virtual Path FindPathTo(PathFindingPoint start, PathFindingPoint goal, const IMap* map) = 0;
};

