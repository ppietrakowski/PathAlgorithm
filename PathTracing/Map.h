#pragma once

#include "MapInterface.h"
#include "PathFindingAlgorithm.h"
#include "LineBatch.h"
#include "RectRenderer.h"

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

    void Draw(const glm::mat4& projection);
    void AddPlayer(glm::ivec2 playerPos);
    void RemovePlayer(glm::ivec2 playerPos);

    bool bVisualizePath = true;

    void DrawPath(const Path& path, size_t startIndex, glm::vec4 color = glm::vec4{1.0f});

    float CellSize = 64.0f;

private:
    std::vector<EFieldType> m_Fields;
    int32_t m_Width;
    int32_t m_Height;
    RectRenderer rectRenderer;
    LineBatch lineBatch;
};