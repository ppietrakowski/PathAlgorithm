#include "PathFindingAlgorithm.h"

#include <queue>

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
    return abs(a.x - b.x) + abs(a.y - b.y);
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

#define NODE_POOL_SIZE 800

class PathFindingData
{
public:
    PathFindingData()
    {
        m_Buffer = (Node*)operator new(sizeof(Node) * NODE_POOL_SIZE);
        m_CurrentAllocationNode = m_Buffer;
    }

    ~PathFindingData() noexcept
    {
        operator delete(m_Buffer);
    }

    Node* AllocateNode(PathFindingPoint point, struct Node* parent)
    {
        assert(m_CurrentAllocationNode - m_Buffer < NODE_POOL_SIZE);
        new (m_CurrentAllocationNode) Node(point, parent);
        return m_CurrentAllocationNode++;
    }

    void StartNewPathFindingSession();

private:
    Node* m_Buffer;
    Node* m_CurrentAllocationNode;
};

static PathFindingData* s_PathFindingData = nullptr;

void PathFindingAlgorithm::Initialize()
{
    s_PathFindingData = new PathFindingData();
}

void PathFindingAlgorithm::Quit()
{
    delete s_PathFindingData;
    s_PathFindingData = nullptr;
}

Path PathFindingAlgorithm::FindPathTo(PathFindingPoint start, PathFindingPoint goal)
{
    /* Find path using A* algorithm */
    AStarProrityQueue openList;
    std::unordered_map<PathFindingPoint, bool> closedList;

    Node startNode(start);
    Node goalNode(goal);
    openList.push(&startNode);

    s_PathFindingData->StartNewPathFindingSession();

    while (!openList.empty())
    {
        Node* currentNode = openList.top();
        openList.pop();
        closedList[currentNode->Point] = true;

        if (currentNode->Point == goal)
        {
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
            if (!IsWalkable(neighbor, IMap::GetInstance().get()) || closedList[neighbor])
            {
                continue;
            }

            Node* neighborNode = s_PathFindingData->AllocateNode(neighbor, currentNode);
            neighborNode->CostFunc = currentNode->CostFunc + 1;
            neighborNode->Heuristics = GetHeuristicsForFields(neighbor, goal);
            neighborNode->EvaluationFunc = neighborNode->CostFunc + neighborNode->Heuristics;

            auto it = std::find_if(openList.begin(), openList.end(), [&](Node* n)
            {
                return n->Point == neighbor;
            });

            if (it == openList.end() || neighborNode->CostFunc < (*it)->CostFunc)
            {
                openList.push(neighborNode);
            }
        }
    }

    return {}; // Return an empty path if no path is found
}

void PathFindingData::StartNewPathFindingSession()
{
    m_CurrentAllocationNode = m_Buffer;
}
