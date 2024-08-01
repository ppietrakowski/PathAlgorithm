#include "GeneticPathFinding.h"
#include "Utils.h"

#include <iostream>

static PathFindingPoint GetRandomNeighbor(PathFindingPoint point, const IMap* map)
{
    PathFindingPoint neighbors[4] = {
        {point.x - 1, point.y}, {point.x + 1, point.y},
        {point.x, point.y - 1}, {point.x, point.y + 1}
    };

    std::shuffle(std::begin(neighbors), std::end(neighbors), Utils::GetGenerator());

    for (const auto& neighbor : neighbors)
    {
        if (IsWalkable(neighbor, map))
        {
            return neighbor;
        }
        else
        {
            std::cout << "Test" << std::endl;
        }
    }

    return point; // No valid neighbor found
}

static Path GetRandomPath(PathFindingPoint start, PathFindingPoint goal, const IMap* map)
{
    Path path;
    path.push_back(start);
    PathFindingPoint current = start;
    while (current != goal)
    {
        current = GetRandomNeighbor(current, map);
        path.push_back(current);
    }
    return path;
}


Path GeneticPathFinding::FindPathTo(PathFindingPoint start, PathFindingPoint goal, const IMap* map)
{
    m_Start = start;
    m_Goal = goal;
    m_Map = map;

    InitializePopulation();

    for (int32_t generation = 0; generation < NumGenerations; ++generation)
    {
        std::sort(m_Population.begin(), m_Population.end(), [](const GeneticPath& a, const GeneticPath& b)
        {
            return a.Fitness > b.Fitness;
        });

        if (m_Population.front().path.back() == m_Goal)
        {
            if (generation == 0)
            {
                continue;
            }

            std::cout << "Path found in generation " << generation << ":\n";

            Path foundPath = std::move(m_Population.front().path);

            for (const auto& point : foundPath)
            {
                std::cout << "(" << point.x << ", " << point.y << ")\n";
            }

            return foundPath;
        }

        EvolvePopulation();
    }

    return Path{};
}

void GeneticPathFinding::InitializePopulation()
{
    m_Population.clear();

    for (int32_t i = 0; i < PopulationSize; ++i)
    {
        m_Population.emplace_back(GetRandomPath(m_Start, m_Goal, m_Map));
        m_Population[i].Fitness = GetFitnessValueForPath(m_Population[i].path);
    }
}

Path GeneticPathFinding::CrossOver(const Path& a, const Path& b)
{
    Path child;
    size_t crossoverPoint = Utils::GetGenerator()() % std::min(a.size(), b.size());
    child.insert(child.end(), a.begin(), a.begin() + crossoverPoint);
    child.insert(child.end(), b.begin() + crossoverPoint, b.end());
    return child;
}

void GeneticPathFinding::MutatePath(Path& path)
{
    for (auto& point : path)
    {
        if (Utils::GetRandomFloat() < MutationRate)
        {
            point = GetRandomNeighbor(point, m_Map);
        }
    }
}

void GeneticPathFinding::EvolvePopulation()
{
    std::vector<GeneticPath> newPopulation;

    for (size_t i = 0; i < m_Population.size(); ++i)
    {
        GeneticPath parent1 = m_Population[Utils::GetGenerator()() % m_Population.size()];
        GeneticPath parent2 = m_Population[Utils::GetGenerator()() % m_Population.size()];
        GeneticPath child;
        child.path = CrossOver(parent1.path, parent2.path);
        MutatePath(child.path);
        child.Fitness = GetFitnessValueForPath(child.path);
        newPopulation.push_back(child);
    }

    m_Population = std::move(newPopulation);
}

double GeneticPathFinding::GetFitnessValueForPath(const Path& path)
{
    double fitness = 0.0;

    if (path.empty())
    {
        return 0.0f;
    }

    for (size_t i = 0; i < path.size() - 1; ++i)
    {
        if (m_Map->GetFieldAt(path[i]) != EFieldType::Empty)
        {
            fitness += 1000; // High penalty for collision
        }

        fitness += glm::distance(glm::vec2(path[i]), glm::vec2(path[i + 1]));
    }

    auto lastPoint = path.back();

    if (m_Map->GetFieldAt(lastPoint) != EFieldType::Empty)
    {
        fitness += 1000;
    }

    fitness += glm::distance(glm::vec2{path.back()}, glm::vec2{m_Goal});
    return 1.0 / (fitness + 1.0); // Higher fitness for shorter paths
}
