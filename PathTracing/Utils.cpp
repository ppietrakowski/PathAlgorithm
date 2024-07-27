#include "Utils.h"

static std::mt19937 s_Generator{std::random_device{}()};

float Utils::GetRandomFloat()
{
    static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(s_Generator);
}

std::mt19937& Utils::GetGenerator()
{
    return s_Generator;
}
