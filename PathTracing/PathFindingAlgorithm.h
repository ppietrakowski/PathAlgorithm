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

class PathFindingAlgorithm
{
public:
    static void Initialize();
    static void Quit();

public:
    static Path FindPathTo(PathFindingPoint start, PathFindingPoint goal);
};