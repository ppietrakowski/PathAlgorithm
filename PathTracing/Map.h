#pragma once

#include "MapInterface.h"
#include <vector>

class Map : public IMap
{
public:
    Map(int32_t width, int32_t height);

    virtual EFieldType GetFieldAt(int32_t x, int32_t y) const override;
    void SetField(int32_t x, int32_t y, EFieldType field);

    bool IsEmpty(int32_t x, int32_t y) const;

    virtual int32_t GetMapWidth() const override;
    virtual int32_t GetMapHeight() const override;

    virtual FieldsByPositionIterator begin() const override;
    virtual FieldsByPositionIterator end() const override;

private:
    std::vector<EFieldType> m_Fields;
    int32_t m_Width;
    int32_t m_Height;
};