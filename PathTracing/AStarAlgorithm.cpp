#include "AStarAlgorithm.h"
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_set>
#include <algorithm>

struct Node
{
    PathFindingPoint Point;
    Node* Parent;
    double CostFunc = 0.0;
    double Heuristics = 0.0;
    double EvaluationFunc = 0.0;

    Node(PathFindingPoint p, Node* parent = nullptr) : 
        Point(p), 
        Parent(parent)
    {
    }
};

static double GetHeuristicsForFields(const PathFindingPoint& a, const PathFindingPoint& b)
{
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

static bool IsWalkable(const PathFindingPoint& point, const IMap* map)
{
    return point.x >= 0 && point.x < map->GetMapWidth() && point.y >= 0 && point.y < map->GetMapHeight() &&  map->GetFieldAt(point.x, point.y) == EFieldType::Empty;
}

struct CompareNode
{
    bool operator()(const Node* a, const Node* b) const
    {
        return a->EvaluationFunc > b->EvaluationFunc;
    }
};

static Path ReconstructPath(Node* node)
{
    Path path;

    /* Traverse nodes from target to start */
    while (node)
    {
        path.push_back(node->Point);
        node = node->Parent;
    }

    std::reverse(path.begin(), path.end());
    return path;
}


struct AStarProrityQueue : public std::priority_queue<Node*, std::vector<Node*>, CompareNode>
{
    container_type& GetContainer()
    {
        return c;
    }

    container_type::iterator begin()
    {
        return c.begin();
    }

    container_type::iterator end()
    {
        return c.end();
    }
};

Path AStarAlgorithm::FindPathTo(PathFindingPoint start, PathFindingPoint goal, const IMap* map)
{
    AStarProrityQueue openList;
    std::unordered_set<PathFindingPoint> closedList;

    Node startNode(start);
    Node goalNode(goal);
    openList.push(&startNode);

    while (!openList.empty())
    {
        Node* currentNode = openList.top();
        openList.pop();
        closedList.insert(currentNode->Point);

        if (currentNode->Point == goal)
        {
            for (auto node : openList)
            {
                if (node != &startNode && node != &goalNode)
                {
                    delete node;
                }
            }

            return ReconstructPath(currentNode);
        }

        PathFindingPoint neighbors[4] = {
            {currentNode->Point.x - 1, currentNode->Point.y},
            {currentNode->Point.x + 1, currentNode->Point.y},
            {currentNode->Point.x, currentNode->Point.y - 1},
            {currentNode->Point.x, currentNode->Point.y + 1}
        };

        for (PathFindingPoint neighbor : neighbors)
        {
            if (!IsWalkable(neighbor, map) || closedList.find(neighbor) != closedList.end())
            {
                continue;
            }

            Node* neighborNode = new Node(neighbor, currentNode);
            neighborNode->CostFunc = currentNode->CostFunc + 1;
            neighborNode->Heuristics = GetHeuristicsForFields(neighbor, goal);
            neighborNode->EvaluationFunc = neighborNode->CostFunc + neighborNode->Heuristics;

            auto it = find_if(openList.begin(), openList.end(), [&](Node* n)
            {
                return n->Point == neighbor;
            });

            if (it == openList.end() || neighborNode->CostFunc < (*it)->CostFunc)
            {
                openList.push(neighborNode);
            }
            else
            {
                delete neighborNode;
            }
        }
    }

    return {}; // Return an empty path if no path is found
}
