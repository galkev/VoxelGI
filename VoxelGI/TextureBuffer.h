#pragma once

#include <glm/glm.hpp>

namespace VoxelGI
{
	template <int numDim, int numColorPlanes>
	class TextureBuffer
	{
	private:
		unsigned char* bufferData;
		glm::ivec3 dim;

		int bufferIndex(int id, int c) const
		{
			return c + numColorPlanes * id;
		}

		int bufferIndex(int x, int y, int z, int c) const
		{
			return c + numColorPlanes * (x + dim.x * (y + dim.y * z));
		}

	public:
		TextureBuffer(int w = 1, int h = 1, int d = 1, int c = 4)
			: bufferData(nullptr)
		{
			resize(w, h, d);
		}

		~TextureBuffer()
		{
			delete[] bufferData;
			bufferData = nullptr;
		}

		constexpr int getNumDim() const
		{
			return numDim;
		}

		constexpr int getNumColorPlanes() const
		{
			return numColorPlanes;
		}

		template <typename P, typename C>
		void setPixel(const P& pos, const C& color)
		{
			for (int i = 0; i < numColorPlanes; i++)
				bufferData[bufferIndex(pos[0], numDim > 1 ? pos[1] : 0, numDim > 2 ? pos[2] : 0, i)] = (unsigned char)(color[i] * 255);
		}

		glm::ivec4 getPixelRaw(const glm::ivec3 pos) const
		{
			glm::vec4 color;

			for (int i = 0; i < numColorPlanes; i++)
				color[i] = bufferData[bufferIndex(pos.x, pos.y, pos.z, i)];

			return color;
		}

		unsigned char* data() const
		{
			return bufferData;
		}

		unsigned int bufferSize() const
		{
			return dim.x * dim.y * dim.z * numColorPlanes * sizeof(unsigned char);
		}

		int pixelCount()
		{
			return dim.x * dim.y * dim.z;
		}

		void resize(int w, int h = 1, int d = 1)
		{
			if (bufferData != nullptr)
				delete bufferData;

			dim.x = w;
			dim.y = h;
			dim.z = d;

			bufferData = new unsigned char[dim.x * dim.y * dim.z * numColorPlanes];
		}

		void clear(const glm::vec4& color)
		{
			for (int i = 0; i < pixelCount(); i++)
			{
				for (int c = 0; c < numColorPlanes; c++)
				{
					bufferData[bufferIndex(i, c)] = (unsigned char)(color[c] * 255);
				}
			}
		}

		const glm::ivec3& size() const
		{
			return dim;
		}
	};
}