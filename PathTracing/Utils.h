#pragma once
#include <random>

class Utils
{
public:
    static float GetRandomFloat();
    static std::mt19937& GetGenerator();
};

