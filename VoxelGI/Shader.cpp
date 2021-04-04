#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

namespace VoxelGI
{

	Shader::Shader()
	{
	}


	Shader::~Shader()
	{
		release();
	}

	void Shader::createFromFile(const std::string& vertexFile, const std::string& fragmentFile)
	{
		GLint result = GL_FALSE;
		int infoLogLength;

		// Create the shaders
		GLuint vertexShaderID = compileShaderFile(ST_VertexShader, vertexFile);
		GLuint fragmentShaderID = compileShaderFile(ST_FragmentShader, fragmentFile);

		if (vertexShaderID == 0 || fragmentShaderID == 0)
		{
			std::cout << "Error creating shader" << std::endl;
			return;
		}

		// Link the program
		printf("Linking program\n");
		programId = glCreateProgram();
		glAttachShader(programId, vertexShaderID);
		glAttachShader(programId, fragmentShaderID);
		glLinkProgram(programId);

		// Check the program
		glGetProgramiv(programId, GL_LINK_STATUS, &result);
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0) {
			std::vector<char> ProgramErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programId, infoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("%s\n", &ProgramErrorMessage[0]);
		}

		glDetachShader(programId, vertexShaderID);
		glDetachShader(programId, fragmentShaderID);

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);

		setupMapping();
	}

	void Shader::release()
	{
		if (programId != 0)
		{
			glDeleteProgram(programId);
			programId = 0;
		}
	}

	GLuint Shader::compileShaderFile(ShaderType shaderType, const std::string& shaderFile)
	{
		GLint result = GL_FALSE;
		int infoLogLength;

		GLuint shaderId = glCreateShader(shaderType == ST_VertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

		std::string shaderCode;
		if (!fileToString(shaderFile, shaderCode))
		{
			std::cout << "Error opening shader \"" << shaderFile << "\"" << std::endl;
			return false;
		}

		// Compile Shader
		printf("Compiling shader : %s\n", shaderFile.c_str());
		char const* sourcePointer = shaderCode.c_str();
		glShaderSource(shaderId, 1, &sourcePointer, NULL);
		glCompileShader(shaderId);

		// Check Shader
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);
			printf("%s\n", &shaderErrorMessage[0]);
		}

		return shaderId;
	}

	bool Shader::fileToString(const std::string& filename, std::string& content)
	{
		std::ifstream filestream(filename);

		if (filestream)
		{
			content = std::string(std::istreambuf_iterator<char>(filestream),
				std::istreambuf_iterator<char>());

			filestream.close();

			return true;
		}
		else
			return false;
	}

	void Shader::bind()
	{
		glUseProgram(programId);
	}

	GLint Shader::getUniformLocation(const std::string& name)
	{
		auto it = uniformMapping.find(name);

		if (it == uniformMapping.end())
		{
			std::cout << "Uniform \"" << name << "\" not found" << std::endl;
			return -1;
		}
		else
			return it->second;
	}

	void Shader::setupMapping()
	{
		const GLsizei bufSize = 32;
		GLsizei length;
		GLchar name[bufSize];
		GLint count;
		GLint size;
		GLenum type;

		glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &count);
		printf("Active Attributes: %d\n", count);

		for (int i = 0; i < count; i++)
		{
			glGetActiveAttrib(programId, (GLuint)i, bufSize, &length, &size, &type, name);

			attributeMapping[name] = i;
		}

		glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &count);

		for (int i = 0; i < count; i++)
		{
			glGetActiveUniform(programId, (GLuint)i, bufSize, &length, &size, &type, name);

			uniformMapping[name] = i;
		}
	}

	void Shader::setUniform(const std::string& name, int value)
	{
		glUniform1i(getUniformLocation(name), value);
	}

	void Shader::setUniform(const std::string & name, const Texture & texture, int slot)
	{
		texture.bind(slot);
		setUniform(name, slot);
	}

	void Shader::setUniform(const std::string& name, const glm::mat4& value)
	{
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void Shader::setUniform(const std::string & name, const glm::vec3 & value)
	{
		glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
	}

	void Shader::setUniform(const std::string & name, const glm::vec2 & value)
	{
		glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
	}

	GLuint Shader::getAttributeIndex(ShaderAttribute attribute)
	{
		return getAttributeIndex(attributeNames[(int)attribute]);
	}

	GLuint Shader::getAttributeIndex(const std::string & name)
	{
		return attributeMapping[name];
	}

	const std::string Shader::attributeNames[] =
	{
		"vertexPos",
		"vertexNormal",
		"vertexTex"
	};

}