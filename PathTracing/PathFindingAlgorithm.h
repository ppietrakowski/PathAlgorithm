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

    virtual void OnUpdate() { }
};


inline bool IsWalkable(const PathFindingPoint& point, const IMap* map)
{
    return point.x >= 0 &&
        point.x < map->GetMapWidth() &&
        point.y >= 0 &&
        point.y < map->GetMapHeight() &&
        (
            map->GetFieldAt(point) == EFieldType::Empty ||
            map->GetFieldAt(point) == EFieldType::Goal
        );
}