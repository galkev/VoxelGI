#pragma once

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

#include <glm\glm.hpp>

#include "Texture.h"

namespace VoxelGI
{

	enum ShaderType
	{
		ST_VertexShader,
		ST_FragmentShader
	};

	class Shader
	{
	private:
		GLuint programId;

		std::unordered_map<std::string, GLint> uniformMapping;
		std::unordered_map<std::string, GLuint> attributeMapping;

		static const std::string attributeNames[];

		bool fileToString(const std::string& filename, std::string& content);
		GLuint compileShaderFile(ShaderType shaderType, const std::string& shaderFile);

		GLint getUniformLocation(const std::string& name);

		void setupMapping();

	public:
		enum ShaderAttribute;

		Shader();
		~Shader();

		void createFromFile(const std::string& vertexFile, const std::string& fragmentFile);

		void release();

		void bind();

		void setUniform(const std::string& name, int value);
		void setUniform(const std::string& name, const Texture& texture, int slot);
		void setUniform(const std::string& name, const glm::mat4& value);
		void setUniform(const std::string& name, const glm::vec3& value);
		void setUniform(const std::string& name, const glm::vec2& value);

		GLuint getAttributeIndex(ShaderAttribute attribute);
		GLuint getAttributeIndex(const std::string& name);
	};

	enum Shader::ShaderAttribute
	{
		SA_Pos,
		SA_Normal,
		SA_Tex,
		NUM_ATTR
	};

}