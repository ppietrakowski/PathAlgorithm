#include "Map.h"
#include "Renderer.h"

glm::vec4 GetColorForField(EFieldType field)
{
    switch (field)
    {
    case EFieldType::Empty:
        return glm::vec4(0.25f);
    case EFieldType::Obstacle:
        return glm::vec4{0.5f, 0.2f, 0.2f, 1.0f};
    case EFieldType::Player:
        return glm::vec4(0.5f, 0.2f, 0.5f, 1.0f);
    case EFieldType::Goal:
        return glm::vec4(0.1f, 0.1f, 0.76f, 1.0f);
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
}


Map::Map(const MapInitialData& initialData):
    Map(initialData.width, initialData.height)
{
}

Map::~Map() noexcept
{
    s_Instance = {};
}

std::shared_ptr<Map> Map::Create(int width, int height)
{
    std::shared_ptr<Map> map = std::make_shared<Map>(MapInitialData{width, height});
    assert(s_Instance.expired());
    s_Instance = map;
    return map;
}

EFieldType Map::GetFieldAt(glm::ivec2 gridPosition) const
{
    return m_Fields[gridPosition.x + gridPosition.y * m_Width];
}

void Map::SetField(glm::ivec2 gridPosition, EFieldType field)
{
    int32_t index = gridPosition.x + gridPosition.y * m_Width;
    m_Fields[index] = field;
}

bool Map::IsEmpty(glm::ivec2 gridPosition) const
{
    return GetFieldAt(gridPosition) == EFieldType::Empty;
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
    for (auto [pos, field] : *this)
    {
        DrawCell(pos, field);
    }
}

void Map::DrawCell(glm::ivec2 pos, EFieldType field)
{
    float posY = static_cast<float>(pos.y);
    float posX = static_cast<float>(pos.x);

    posY *= CellSize;
    posX *= CellSize;

    glm::vec4 color = GetColorForField(field);

    /* For players just draw empty rect in background first */
    if (field == EFieldType::Player)
    {
        color = GetColorForField(EFieldType::Empty);
    }

    /* Render bounds first */
    Renderer::DrawRect(glm::vec3{posX, posY, -1.0f},
        glm::vec3{CellSize, CellSize, 0.0f}, DrawCommandArgs{color * 0.4f});

    /* Now render right field */
    Renderer::DrawRect(glm::vec3{posX + 2.5, posY + 2.5, -1.0f},
        glm::vec3{CellSize - 5, CellSize - 5, 0.0f},
        DrawCommandArgs{color});
}

float Map::GetCellSize() const
{
    return CellSize;
}
