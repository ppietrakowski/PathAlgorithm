#include "Map.h"

Map::Map(int32_t width, int32_t height) :
    m_Fields(static_cast<size_t>(width* height), EFieldType::Empty),
    m_Width(width),
    m_Height(height)
{
}

EFieldType Map::GetFieldAt(int32_t x, int32_t y) const
{
    return m_Fields[x + y * m_Width];
}

void Map::SetField(int32_t x, int32_t y, EFieldType field)
{
    m_Fields[x + y * m_Width] = field;
}

bool Map::IsEmpty(int32_t x, int32_t y) const
{
    return GetFieldAt(x, y) == EFieldType::Empty;
}

int32_t Map::GetMapWidth() const
{
    return m_Width;
}

int32_t Map::GetMapHeight() const
{
    return m_Height;
}

FieldsByPositionIterator Map::begin() const
{
    return FieldsByPositionIterator{this, glm::ivec2(0, 0)};
}

FieldsByPositionIterator Map::end() const
{
    return FieldsByPositionIterator{this, glm::ivec2(0, m_Height)};
}