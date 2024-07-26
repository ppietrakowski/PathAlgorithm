#pragma once

#include <cstdint>
#include <glm/glm.hpp>

enum class EFieldType : uint8_t
{
    Empty = 0,
    Obstacle,
    Player,
    Max
};

class FieldsByPositionIterator
{
public:
    FieldsByPositionIterator(const class IMap* map, glm::ivec2 pos);

    bool operator==(const FieldsByPositionIterator& i) const
    {
        return i.m_Pos == m_Pos;
    }

    bool operator!=(const FieldsByPositionIterator& i) const
    {
        return i.m_Pos != m_Pos;
    }

    FieldsByPositionIterator operator++(int) const;
    FieldsByPositionIterator& operator++();

    std::pair<glm::ivec2, EFieldType> operator*() const;

private:
    const class IMap* m_Map;
    glm::ivec2 m_Pos;
};

class IMap
{
public:
    virtual ~IMap() = default;

    virtual EFieldType GetFieldAt(int32_t x, int32_t y) const = 0;
    virtual int32_t GetMapWidth() const = 0;
    virtual int32_t GetMapHeight() const = 0;

    virtual FieldsByPositionIterator begin() const = 0;
    virtual FieldsByPositionIterator end() const = 0;
};