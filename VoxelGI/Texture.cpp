#include "Texture.h"

#include <iostream>

namespace VoxelGI
{

	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
		if (textureName != 0)
			glDeleteTextures(1, &textureName);

		std::cout << "Texture destroyed" << std::endl;
	}

	void Texture::bind(int slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(textureType, textureName);
	}

	void Texture::createFromBuffer(GLenum texType, const glm::ivec3 & dim, const void * buffer)
	{
		//createFromBuffer(texType, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, dim, buffer);

		textureType = texType;

		if (textureName == 0)
			glGenTextures(1, &textureName);

		switch (textureType)
		{
		case GL_TEXTURE_2D:
			glBindTexture(GL_TEXTURE_2D, textureName);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.x, dim.y, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, buffer);

		case GL_TEXTURE_3D:
			glBindTexture(GL_TEXTURE_3D, textureName);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, dim.x, dim.y, dim.z, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, buffer);

			break;
		}
	}

	void Texture::createFromBuffer(GLenum texType, GLint internalFormat, GLenum format, GLenum pixelType, GLenum texFilter, const glm::ivec3 & dim, const void * buffer)
	{
		textureType = texType;

		if (textureName == 0)
			glGenTextures(1, &textureName);

		glBindTexture(textureType, textureName);
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, texFilter);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, texFilter);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		switch (textureType)
		{
		case GL_TEXTURE_2D:
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dim.x, dim.y, 0, format,
				pixelType, buffer);
			break;
		case GL_TEXTURE_3D:
			glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, dim.x, dim.y, dim.z, 0, format,
				pixelType, buffer);
			break;
		}
	}

	bool Texture::valid()
	{
		return textureName != 0;
	}

}