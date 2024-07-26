#include "MapInterface.h"

FieldsByPositionIterator::FieldsByPositionIterator(const IMap* map, glm::ivec2 pos) :
    m_Map(map),
    m_Pos(pos)
{
}

FieldsByPositionIterator FieldsByPositionIterator::operator++(int) const
{
    glm::ivec2 pos = m_Pos;
    pos.x++;

    if (pos.x >= m_Map->GetMapWidth())
    {
        pos.x = 0;
        pos.y++;
    }

    return FieldsByPositionIterator{m_Map, pos};
}

FieldsByPositionIterator& FieldsByPositionIterator::operator++()
{
    m_Pos.x++;

    if (m_Pos.x >= m_Map->GetMapWidth())
    {
        m_Pos.x = 0;
        m_Pos.y++;
    }

    return *this;
}

std::pair<glm::ivec2, EFieldType> FieldsByPositionIterator::operator*() const
{
    return {m_Pos, m_Map->GetFieldAt(m_Pos.x, m_Pos.y)};
}
