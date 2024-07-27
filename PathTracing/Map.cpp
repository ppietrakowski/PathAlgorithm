#include "Map.h"

static glm::vec4 GetColorForField(EFieldType field)
{
    switch (field)
    {
    case EFieldType::Empty:
        return glm::vec4(0.25f);
    case EFieldType::Obstacle:
        return glm::vec4{0.5f, 0.2f, 0.2f, 1.0f};
    default:
        break;
    }
    return glm::vec4{0.0f};
}

Map::Map(int32_t width, int32_t height) :
    m_Fields(static_cast<size_t>(width* height), EFieldType::Empty),
    m_Width(width),
    m_Height(height)
{
    lineBatch.Thickness = 2.0f;
}

EFieldType Map::GetFieldAt(int32_t x, int32_t y) const
{
    return m_Fields[x + y * m_Width];
}

void Map::SetField(int32_t x, int32_t y, EFieldType field)
{
    int32_t index = x + y * m_Width;
    m_Fields[index] = field;
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


void Map::Draw(const glm::mat4& projection)
{
    rectRenderer.UpdateProjection(projection);
    lineBatch.OnBeginScene(projection);

    for (auto [pos, field] : *this)
    {
        float posY = pos.y;
        float posX = pos.x;

        posY *= CellSize;
        posX *= CellSize;

        glm::vec4 color = GetColorForField(field);
        color *= 0.4f;

        /* Render bounds first */
        rectRenderer.AddRectInstance(glm::vec3{posX, posY, -1.0f},
            glm::vec3{CellSize, CellSize, 0.0f}, color);

        /* Now render right field */
        rectRenderer.AddRectInstance(glm::vec3{posX + 2.5, posY + 2.5, -1.0f},
            glm::vec3{CellSize - 5, CellSize - 5, 0.0f},
            GetColorForField(field));
    }

    rectRenderer.FlushDraw();
    lineBatch.FlushDraw();
}

void Map::AddPlayer(glm::ivec2 playerPos)
{
    SetField(playerPos.x, playerPos.y, EFieldType::Player);
}

void Map::RemovePlayer(glm::ivec2 playerPos)
{
    SetField(playerPos.x, playerPos.y, EFieldType::Empty);
}

void Map::DrawPath(const Path& path, size_t startIndex, glm::vec4 color)
{
    for (size_t i = 0; path.size() > 0 && i < path.size() - 1; ++i)
    {
        glm::vec3 pos = glm::vec3{path[i], 0};
        glm::vec3 nextpos = glm::vec3{path[i + 1], 0};

        for (int j = 0; j < pos.length(); ++j)
        {
            pos[j] *= CellSize;
            nextpos[j] *= CellSize;
        }

        pos.x += CellSize / 2;
        pos.y += CellSize / 2;
        nextpos.x += CellSize / 2;
        nextpos.y += CellSize / 2;

        lineBatch.DrawLine(pos, nextpos, glm::mat4{1.0f}, color);
    }
}