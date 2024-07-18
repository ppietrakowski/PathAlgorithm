#include "Shader.h"
#include <cassert>

#include "UniformBuffer.h"

#include <fstream>
#include <sstream>
#include <span>

#include <glad/glad.h>
#include <filesystem>

static bool ReadFileContent(const std::string& filepath, std::string& source)
{
    std::ifstream file(filepath);

    if (!file)
    {
        puts("Core: invalid file: cannot access");
        return false;
    }

    std::ostringstream os;
    os << file.rdbuf();
    source = os.str();
    return true;
}

static constexpr std::string_view VertexShaderSourceTag = "#vertex";
static constexpr std::string_view FragmentShaderSourceTag = "#fragment";

inline static void Trim(std::string& s)
{
    size_t indexOfWhiteSpace = s.find_first_not_of("\t\r\n\f\v ");

    if (indexOfWhiteSpace != std::string::npos)
    {
        s = s.substr(indexOfWhiteSpace);
    }

    indexOfWhiteSpace = s.find_last_not_of("\t\r\n\f\v ");

    if (indexOfWhiteSpace != std::string::npos)
    {
        s = s.substr(0, indexOfWhiteSpace);
    }
}

static bool GetShaderSourcesFromPath(const std::string& filepath, ShaderSource& outSource)
{
    std::string source;

    if (!ReadFileContent(filepath, source))
    {
        return false;
    }

    size_t vertexShaderTagPos = source.find(VertexShaderSourceTag);
    size_t fragmentShaderTagPos = source.find(FragmentShaderSourceTag);

    if (vertexShaderTagPos == std::string::npos || fragmentShaderTagPos == std::string::npos)
    {
        if (vertexShaderTagPos == std::string::npos)
        {
            puts("Renderer: #vertex tag couldn't be found");
        }
        else
        {
            puts("Renderer: #fragment tag couldn't be found");
        }

        return false;
    }

    size_t vertexLetterCount = fragmentShaderTagPos - VertexShaderSourceTag.length() - vertexShaderTagPos;
    std::string vertexShaderSrc = source.substr(vertexShaderTagPos + VertexShaderSourceTag.length(), vertexLetterCount);

    size_t start = fragmentShaderTagPos + FragmentShaderSourceTag.length();
    std::string fragmentShaderSrc = source.substr(start);
    outSource = ShaderSource{vertexShaderSrc, fragmentShaderSrc};
    return true;
}


struct OpenGLShaderObject
{
    GLuint ShaderObject = 0;
    GLuint ProgramID = 0;

    OpenGLShaderObject() = default;
    OpenGLShaderObject(GLuint shader) noexcept :
        ShaderObject(shader)
    {
    }

    OpenGLShaderObject(OpenGLShaderObject&& object) noexcept
    {
        object = std::exchange(object.ShaderObject, 0);
        ProgramID = std::exchange(object.ProgramID, 0);
    }

    OpenGLShaderObject& operator=(OpenGLShaderObject&& object) noexcept
    {
        ShaderObject = std::exchange(object.ShaderObject, 0);
        ProgramID = std::exchange(object.ProgramID, 0);

        return *this;
    }

    ~OpenGLShaderObject() noexcept
    {
        if (ProgramID)
        {
            glDetachShader(ProgramID, ShaderObject);
        }
        else
        {
            glDeleteShader(ShaderObject);
        }
    }

    operator bool() const
    {
        return ShaderObject != 0;
    }

    bool operator!() const
    {
        return ShaderObject == 0;
    }

    void AttachShader(GLuint program)
    {
        glAttachShader(program, ShaderObject);
        ProgramID = program;
    }
};

// Size of buffer of info log returned by shader
// Error message should never be longer than that
constexpr GLint InfoLogBufferSize = 2048;

static const char* GetShaderTypeName(GLenum type)
{
    switch (type)
    {
    case GL_VERTEX_SHADER:
        return "vertex";
    case GL_FRAGMENT_SHADER:
        return "fragment";
    default:
        break;
    }

    return "";
}

static void BuildCompileShaderErrorLog(GLuint shader, GLenum type, std::span<char> outBuffer)
{
    GLchar messageBuf[InfoLogBufferSize];
    GLint len = InfoLogBufferSize;

    glGetShaderInfoLog(shader, len, &len, messageBuf);
    messageBuf[InfoLogBufferSize - 1] = 0;

    snprintf(outBuffer.data(), outBuffer.size(), "%s shader compilation error: %s\n", GetShaderTypeName(type), messageBuf);
    outBuffer.back() = '\0';
}

static GLuint CreateShaderObject(std::string_view source, GLenum type)
{
    GLuint shaderObject;
    shaderObject = glCreateShader(type);

    const char* src = source.data();

    GLint length;
    length = static_cast<GLint>(source.length());

    glShaderSource(shaderObject, 1, &src, &length);
    glCompileShader(shaderObject);

    GLint compilationGood;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compilationGood);

    if (!compilationGood)
    {
        char messageBuffer[InfoLogBufferSize];
        BuildCompileShaderErrorLog(shaderObject, type, messageBuffer);
        glDeleteShader(shaderObject);
        printf("Renderer: %s\n", messageBuffer);
        return 0;
    }

    return shaderObject;
}

namespace GLShaderConstants
{
    enum Type
    {
        VertexShaderIndex = 0,
        FragmentShaderIndex,
        NumShaders
    };

    static GLenum ToGLEnum(int32_t type)
    {
        switch (type)
        {
        case GLShaderConstants::VertexShaderIndex:
            return GL_VERTEX_SHADER;
        case GLShaderConstants::FragmentShaderIndex:
            return GL_FRAGMENT_SHADER;
        case GLShaderConstants::NumShaders:
            break;
        default:
            break;
        }

        assert(0 && "Invalid enum");
        return 0;
    }
}

static GLuint CreateOpenGLProgramFromSource(std::string_view vertexSource, std::string_view fragmentSource)
{
    OpenGLShaderObject shaderObjects[GLShaderConstants::NumShaders];
    GLuint programId = glCreateProgram();

    std::string_view sources[GLShaderConstants::NumShaders];
    sources[0] = vertexSource;
    sources[1] = fragmentSource;

    for (int32_t i = GLShaderConstants::VertexShaderIndex; i < GLShaderConstants::NumShaders; ++i)
    {
        shaderObjects[i] = CreateShaderObject(sources[i], GLShaderConstants::ToGLEnum(i));

        if (!shaderObjects[i])
        {
            glDeleteShader(programId);
            return 0;
        }

        shaderObjects[i].AttachShader(programId);
    }

    glLinkProgram(programId);

    GLint linkingPerfomedSuccesfully;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkingPerfomedSuccesfully);

    if (!linkingPerfomedSuccesfully)
    {
        GLchar buffer[InfoLogBufferSize];
        GLint len = InfoLogBufferSize;

        glGetProgramInfoLog(programId, len, &len, buffer);
        buffer[InfoLogBufferSize - 1] = 0;

        printf("Renderer: shader linking error: %s", buffer);
        return 0;
    }

    return programId;
}

Shader::Shader(const ShaderInitialData& initialData) :
    m_ProgramId(initialData.ProgramID),
    m_Name(initialData.Name)
{
}

Shader::~Shader() noexcept
{
    glDeleteProgram(m_ProgramId);
}

void Shader::Bind() const
{
    glUseProgram(m_ProgramId);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::SetInt(const char* name, int32_t value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetIntArray(const char* name, std::span<const int32_t> values)
{
    static_assert(sizeof(int32_t) == sizeof(GLint));

    glUniform1iv(GetUniformLocation(name), (int32_t)values.size(), values.data());
}

void Shader::SetFloat(const char* name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetFloat2(const char* name, const glm::vec2& value)
{
    glUniform2fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetFloat3(const char* name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetFloat4(const char* name, const glm::vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetMat4(const char* name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::SetMat4Array(const char* name, std::span<const glm::mat4> values)
{
    glUniformMatrix4fv(GetUniformLocation(name),
        static_cast<GLsizei>(values.size()), GL_FALSE, &values[0][0][0]);
}

void Shader::SetMat3(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::BindUniformBuffer(const char* blockName, const UniformBuffer& buffer)
{
    GLint blockIndex = GetUniformBlockIndex(blockName);

    if (blockIndex != -1)
    {
        buffer.Bind(blockIndex);
        glUniformBlockBinding(m_ProgramId, blockIndex, blockIndex);
    }
}

bool Shader::HasUniformBlock(const char* blockName) const
{
    int32_t location = GetUniformBlockIndex(blockName);
    return location != -1;
}

const std::string& Shader::GetName() const
{
    return m_Name;
}

static inline EShaderDataType GlUniformTypeToPlaint32_type(GLenum type)
{
    switch (type)
    {
    case GL_FLOAT:
        return EShaderDataType::Float;
    case GL_INT:
        return EShaderDataType::Int;
    case GL_FLOAT_VEC2:
        return EShaderDataType::Vec2;
    case GL_FLOAT_VEC3:
        return EShaderDataType::Vec3;
    case GL_FLOAT_VEC4:
        return EShaderDataType::Vec4;
    case GL_FLOAT_MAT4:
        return EShaderDataType::Mat4;
    case GL_FLOAT_MAT3:
        return EShaderDataType::Mat3;
    case GL_BOOL:
        return EShaderDataType::Boolean;
    case GL_SAMPLER_2D:
        return EShaderDataType::Sampler2D;
    default:
        break;
    }

    return EShaderDataType::None;
}

std::vector<UniformInfo> Shader::GetUniformsInfo() const
{
    assert(m_ProgramId != 0 && "Trying to access materials from NULL shader");

    GLint numUniforms;
    glGetProgramiv(m_ProgramId, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::vector<UniformInfo> uniformInfos;
    uniformInfos.reserve(numUniforms);

    constexpr int32_t MaxNameLength = 96;

    for (GLint location = 0; location < numUniforms; ++location)
    {
        GLchar name[MaxNameLength]; // variable name in GLSL
        GLsizei nameLength; // name length
        GLint size; // size of the variable
        GLenum type; // type of the variable (float, vec3 or mat4, etc)

        glGetActiveUniform(m_ProgramId, static_cast<GLuint>(location), MaxNameLength, &nameLength, &size, &type, name);
        EShaderDataType convertedType = GlUniformTypeToPlaint32_type(type);

        if (convertedType != EShaderDataType::None)
        {
            UniformInfo info{convertedType, size, location, name};
            uniformInfos.emplace_back(info);
        }
    }

    return uniformInfos;
}

int32_t Shader::GetUniformLocation(const char* name) const
{
    auto uniformLocation = m_UniformNameToIndex.find(name);

    if (uniformLocation == m_UniformNameToIndex.end())
    {
        GLint location = glGetUniformLocation(m_ProgramId, name);
        m_UniformNameToIndex.try_emplace(name, location);
        return location;
    }

    return uniformLocation->second;
}

int32_t Shader::GetUniformBlockIndex(const char* name) const
{
    auto index = m_UniformNameToIndex.find(name);

    if (index == m_UniformNameToIndex.end())
    {
        GLint location = glGetUniformBlockIndex(m_ProgramId, name);
        m_UniformNameToIndex[name] = location;
        return location;
    }

    return index->second;
}

bool Shader::Create(const std::string& filepath, std::shared_ptr<Shader>& outShader)
{
    ShaderSource source;

    if (!GetShaderSourcesFromPath(filepath, source))
    {
        return false;
    }

    return Create(filepath, source, outShader);
}

bool Shader::Create(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, std::shared_ptr<Shader>& outShader)
{
    ShaderSource source;

    if (!ReadFileContent(vertexShaderPath, source.VertexShaderSource))
    {
        return false;
    }

    if (!ReadFileContent(fragmentShaderPath, source.FragmentShaderSource))
    {
        return false;
    }

    std::filesystem::path path = vertexShaderPath;
    path = path.stem();

    return Create(path.string(), source, outShader);
}

bool Shader::Create(const std::string& name, const ShaderSource& shaderSource, std::shared_ptr<Shader>& outShader)
{
    GLuint programID = CreateOpenGLProgramFromSource(shaderSource.VertexShaderSource, shaderSource.FragmentShaderSource);

    if (!programID)
    {
        return false;
    }

    outShader = std::make_shared<Shader>(ShaderInitialData{programID, name});
    return true;
}
