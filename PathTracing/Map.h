#pragma once

#include "MapInterface.h"
#include "PathFindingAlgorithm.h"

#include <vector>


class Map : public IMap
{
    struct MapInitialData
    {
        int width;
        int height;
    };
public:
    Map(const MapInitialData& initialData);
    ~Map() noexcept;

    static std::shared_ptr<Map> Create(int width, int height);

public:
    virtual EFieldType GetFieldAt(glm::ivec2 gridPosition) const override;
    virtual void SetField(glm::ivec2 gridPosition, EFieldType field) override;

    bool IsEmpty(glm::ivec2 gridPosition) const;

    virtual int32_t GetMapWidth() const override;
    virtual int32_t GetMapHeight() const override;

    virtual FieldsByPositionIterator begin() const override;
    virtual FieldsByPositionIterator end() const override;

    void Draw(const glm::mat4& projection);

    bool bVisualizePath = true;

    void DrawPath(const Path& path, size_t startIndex, glm::vec2 startPos, glm::vec4 color = glm::vec4{1.0f});

    virtual float GetCellSize() const override;

private:
    Map(int32_t width, int32_t height);

private:
    std::vector<EFieldType> m_Fields;
    int32_t m_Width;
    int32_t m_Height;
    float CellSize = 64.0f;
};

glm::vec4 GetColorForField(EFieldType field);