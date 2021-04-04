#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "TextureBuffer.h"

namespace VoxelGI
{

	class Texture
	{
	private:
		GLenum textureType;
		GLuint textureName;

	public:
		Texture();
		~Texture();

		void bind(int slot) const;

		template <typename T>
		void createFromBuffer(const T& buffer, GLenum texFilter = GL_NEAREST);

		void createFromBuffer(GLenum texType, const glm::ivec3& dim, const void* buffer);
		void createFromBuffer(GLenum texType, GLint internalFormat, GLenum format, GLenum pixelType, GLenum texFilter, const glm::ivec3 & dim, const void * buffer);

		bool valid();
	};

	template <typename T>
	inline void Texture::createFromBuffer(const T& buffer, GLenum texFilter)
	{
		GLenum texType = buffer.getNumDim() == 3 ? GL_TEXTURE_3D : GL_TEXTURE_2D;

		GLint internalFormat;
		GLenum format;
		GLenum pixelType;

		switch (buffer.getNumColorPlanes())
		{
		case 3:
			internalFormat = GL_RGB;
			format = GL_RGB;
			break;
		case 4:
			internalFormat = GL_RGBA;
			format = GL_RGBA;
			break;
		}

		pixelType = GL_UNSIGNED_BYTE;

		//createFromBuffer(texType, dim, buffer.data());
		createFromBuffer(texType, internalFormat, format, pixelType, texFilter, buffer.size(), buffer.data());
	}

}