#pragma once

#include "PathFindingAlgorithm.h"
#include <utility>

struct GeneticPath
{
    Path path;
    double Fitness = 0.0f;

    GeneticPath() = default;
    GeneticPath(Path&& path) :
        path(std::move(path))
    {
    }
};

class GeneticPathFinding : public IPathFindingAlgorithm
{
public:
    int32_t PopulationSize = 100;
    int32_t NumGenerations = 1000;
    float MutationRate = 0.01f;

    virtual Path FindPathTo(PathFindingPoint start, PathFindingPoint goal, const IMap* map) override;

private:
    std::vector<GeneticPath> m_Population;
    const IMap* m_Map{nullptr};
    PathFindingPoint m_Start;
    PathFindingPoint m_Goal;

    void InitializePopulation();

    Path CrossOver(const Path& a, const Path& b);
    void MutatePath(Path& path);

    void EvolvePopulation();

private:
    double GetFitnessValueForPath(const Path& path);
};

