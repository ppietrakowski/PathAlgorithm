#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

enum class EFieldType : uint8_t
{
    Empty = 0,
    Obstacle,
    Player,
    Goal,
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

class IMap : public std::enable_shared_from_this<IMap>
{
public:
    IMap() = default;
    virtual ~IMap() noexcept = default;

    static std::shared_ptr<IMap> GetInstance();

    virtual EFieldType GetFieldAt(glm::ivec2 gridPosition) const = 0;
    virtual void SetField(glm::ivec2 gridPosition, EFieldType field) = 0;

    virtual int32_t GetMapWidth() const = 0;
    virtual int32_t GetMapHeight() const = 0;

    virtual FieldsByPositionIterator begin() const = 0;
    virtual FieldsByPositionIterator end() const = 0;

    virtual float GetCellSize() const = 0;

protected:
    static std::weak_ptr<IMap> s_Instance;
};