#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <span>

enum class EShaderDataType : uint8_t
{
	None = 0, Float,
	Vec2, Vec3,
	Vec4, Mat3, Mat4,
	Int, Boolean,
	Sampler2D
};

struct UniformInfo
{
	EShaderDataType DataType{EShaderDataType::None};
	int32_t Size{1};
	int32_t Location{-1};
	std::string UniformName;
};

struct ShaderSource
{
	std::string VertexShaderSource;
	std::string FragmentShaderSource;
};

class Shader
{
private:
	// struct just for enforcing only private construction
	struct ShaderInitialData
	{
		uint32_t ProgramID;
		std::string Name;
	};

public:
	Shader(const ShaderInitialData& initialData);
	~Shader() noexcept;

	static [[nodiscard]] bool Create(const std::string& filepath, std::shared_ptr<Shader>& outShader);
	static [[nodiscard]] bool Create(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, std::shared_ptr<Shader>& outShader);
	static [[nodiscard]] bool Create(const std::string& name, const ShaderSource& shaderSource, std::shared_ptr<Shader>& outShader);

public:
	void Bind() const;
	void Unbind() const;

	void SetInt(const char* name, int32_t value);
	void SetIntArray(const char* name, std::span<const int32_t> values);
	void SetFloat(const char* name, float value);

	void SetFloat2(const char* name, const glm::vec2& value);
	void SetFloat3(const char* name, const glm::vec3& value);
	void SetFloat4(const char* name, const glm::vec4& value);

	void SetMat4(const char* name, const glm::mat4& value);
	void SetMat4Array(const char* name, std::span<const glm::mat4> values);
	void SetMat3(const char* name, const glm::mat3& value);

	void BindUniformBuffer(const char* blockName, const class UniformBuffer& buffer);
	bool HasUniformBlock(const char* blockName) const;

	const std::string& GetName() const;
	std::vector<UniformInfo> GetUniformsInfo() const;

private:
	uint32_t m_ProgramId{};
	std::string m_Name;

	/* Cached uniform name to index just to fast return location */
	mutable std::unordered_map<std::string, int32_t> m_UniformNameToIndex;

private:
	int32_t GetUniformLocation(const char* name) const;
	int32_t GetUniformBlockIndex(const char* name) const;
};