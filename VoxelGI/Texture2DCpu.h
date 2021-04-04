#pragma once

#include <vector>
#include <iostream>
#include <glm\glm.hpp>
#include <string>

#include "lodepng\lodepng.h"
#include "MultiBuffer.h"

namespace VoxelGI
{
	class Boundary
	{
	private:
		template <typename T>
		static bool outOfBounds(T val, T lower, T upper)
		{
			return val < lower || val > upper;
		}

	public:
		template <typename T>
		static T clamp(T val, T lower, T upper)
		{
			if (val > upper)
				return upper;
			else if (val < lower)
				return lower;
			else
				return val;
		}

		template <typename T>
		static T periodic(T val, T lower, T upper)
		{
			if (!outOfBounds(val, lower, upper))
				return val;

			T res = (val - lower) % (upper - lower);

			if (res < 0)
				res += upper - lower;

			return res + lower;
		}

		template <typename T>
		static T reflective(T val, T lower, T upper)
		{
			if (!outOfBounds(val, lower, upper))
				return val;
			
			int reflectIndex = (val - lower) / (upper - lower);

			if (reflectIndex & 0x1 == 0)
				return periodic(val, lower, upper);
			else if (reflectIndex < 0)
				return periodic(lower - (val - lower), lower, upper);
			else
				return periodic(upper - (val - upper), lower, upper);
		}
	};

	template <typename T>
	class Texture2DCpu
	{
	protected:
		enum FilterMode
		{
			FM_Nearest,
			FM_Linear
		};

		using vec2 = glm::tvec2<T>;
		using vec4 = glm::tvec4<T>;

		std::vector<unsigned char> image;

		glm::uvec2 dim;

		FilterMode filterMode = FM_Linear;

	public:
		Texture2DCpu() {}

		void resize(int dimX, int dimY)
		{
			dim.x = dimX;
			dim.y = dimY;

			image.resize(4 * dim.x * dim.y);
		}

		glm::uvec2 size()
		{
			return dim;
		}

		virtual void load(const std::string& filename)
		{
			auto error = lodepng::decode(image, dim.x, dim.y, filename.c_str());

			if (error) 
				std::cout << "texture decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		}

		void save(const std::string& filename)
		{
			auto error = lodepng::encode(filename, image, dim.x, dim.y);

			if (error)
				std::cout << "texture encode error " << error << ": " << lodepng_error_text(error) << std::endl;
		}

		virtual void saveMipMaps(const std::string& filename)
		{
			save(filename);
		}

		void setPixelRaw(const glm::ivec2& pos, const glm::uvec4& val)
		{
			setPixelRaw(pos.x, pos.y, val);
		}

		void setPixelRaw(int x, int y, const glm::uvec4& val)
		{
			int bufferPos = 4 * (y * dim.x + x);

			image[bufferPos + 0] = val[0];
			image[bufferPos + 1] = val[1];
			image[bufferPos + 2] = val[2];
			image[bufferPos + 3] = val[3];
		}

		glm::uvec4 getPixelRaw(const glm::ivec2& pos) const
		{
			return getPixelRaw(pos.x, pos.y);
		}

		glm::uvec4 getPixelRaw(int x, int y) const
		{
			int bufferPos = 4 * (y * dim.x + x);

			return glm::uvec4(
				image[bufferPos + 0],
				image[bufferPos + 1],
				image[bufferPos + 2],
				image[bufferPos + 3]
			);
		}

		glm::vec4 getPixel(const glm::ivec2& pos) const
		{
			return glm::vec4(getPixelRaw(pos)) / 255.0f;
		}

		// pos in [0, 1]
		glm::vec4 sample(const vec2& pos) const
		{
			vec2 posBuffer = vec2(pos.x, (T)1 - pos.y) * vec2(dim - glm::uvec2(1));

			if (filterMode == FM_Nearest)
			{
				return getPixel(glm::clamp(glm::ivec2(glm::round(posBuffer - vec2(0.5))), glm::ivec2(0), glm::ivec2(dim - glm::uvec2(1))));
			}
			else if (filterMode == FM_Linear)
			{
				glm::ivec2 cell = glm::ivec2(glm::floor(posBuffer - vec2(0.5f)));

				vec2 t = posBuffer - (vec2(cell) + vec2(0.5));

				glm::tvec4<T> v[4] =
				{
					getPixel(glm::clamp(glm::ivec2(cell.x + 0, cell.y + 0), glm::ivec2(0), glm::ivec2(dim - glm::uvec2(1)))),
					getPixel(glm::clamp(glm::ivec2(cell.x + 1, cell.y + 0), glm::ivec2(0), glm::ivec2(dim - glm::uvec2(1)))),
					getPixel(glm::clamp(glm::ivec2(cell.x + 0, cell.y + 1), glm::ivec2(0), glm::ivec2(dim - glm::uvec2(1)))),
					getPixel(glm::clamp(glm::ivec2(cell.x + 1, cell.y + 1), glm::ivec2(0), glm::ivec2(dim - glm::uvec2(1))))
				};

				return glm::vec4((1 - t.y) * ((1 - t.x) * v[0] + t.x * v[1]) + t.y * ((1 - t.x) * v[2] + t.x * v[3]));
			}
		}

		virtual glm::vec4 sample(const vec2& pos, T lod) const
		{
			return sample(pos);
		}

		template <typename V>
		void toVecBuffer(Buffer2D<V>& buffer) const
		{
			buffer.resize(dim.x, dim.y);

			for (int y = 0; y < dim.y; y++)
			{
				for (int x = 0; x < dim.x; x++)
				{
					buffer(x, y) = V(getPixelRaw(x, y));
				}
			}
		}

		template <typename V>
		void fromVecBuffer(const Buffer2D<V>& buffer)
		{
			resize(buffer.sizeX(), buffer.sizeY());

			for (int y = 0; y < dim.y; y++)
			{
				for (int x = 0; x < dim.x; x++)
				{
					/*for (int i = 0; i < 4; i++)
					{
						if (buffer(x, y)[i] < 0.0f)
						{
							std::cout << glm::to_string(buffer(x, y)) << std::endl;
							break;
						}
						else if (buffer(x, y)[i] > 255.0f)
						{
							std::cout << glm::to_string(buffer(x, y)) << std::endl;
							break;
						}
					}*/

					setPixelRaw(x, y, 
						glm::uvec4(glm::clamp(glm::ivec4(glm::round(buffer(x, y))), glm::ivec4(0), glm::ivec4(0xff))));
				}
			}
		}
	};


	template <typename T>
	class TextureMip2DCpu : public Texture2DCpu<T>
	{
	public:
		enum MipMapFilter
		{
			MMF_Point,
			MMF_Box,
			MMF_Gaussian,
			MMF_Kaiser,
			MMF_Lanczos
		};

	private:
		std::vector<Texture2DCpu> mipMaps;

		int maxPotentialLod()
		{
			return std::log2(std::max(dim.x, dim.y));
		}

		void generateMipMaps()
		{
			generateMipMaps(MMF_Kaiser, maxPotentialLod());
		}

		double sinc(double x) 
		{
			if (x == 0.0)
				return 1.0;
			
			return std::sin(glm::pi<double>() * x) / (glm::pi<double>() * x);
		}

		double bessel0(double x)
		{
			const double EPSILON_RATIO = 1E-16;
			double xh, sum, pow, ds;

			xh = 0.5 * x;
			sum = 1.0;
			pow = 1.0;
			ds = 1.0;

			int k = 0;
			while (ds > sum * EPSILON_RATIO) {
				k++;
				pow *= (xh / k);
				ds = pow * pow;
				sum = sum + ds;
			}

			return sum;
		}

		double kaiser(double alpha, double halfWidth, double x) 
		{
			double ratio = (x / halfWidth);
			return bessel0(alpha * std::sqrt(1 - ratio * ratio)) / bessel0(alpha);
		}

		void getKaiserFilter(
			int filterWidth, 
			double alpha, 
			double additionalStretch, 
			std::vector<float>& filter)
		{
			filter.resize(filterWidth);

			float halfWidth = (float)(filterWidth / 2);
			float offset = -halfWidth;
			float nudge = 0.5f;

			float stretch = additionalStretch;

			for (int i = 0; i < filterWidth; i++)
			{
				float x = (i + offset) + nudge;

				double sincValue = sinc(x * stretch);
				double windowValue = kaiser(alpha, halfWidth, x * stretch);

				filter[i] = sincValue * windowValue;
			}
		}

		void getLanczosFilter(
			int filterWidth,
			double additionalStretch,
			double extra1,
			double extra2,
			std::vector<float>& filter)
		{
			filter.resize(filterWidth);

			float halfWidth = (float)(filterWidth / 2);
			float offset = -halfWidth;
			float nudge = 0.5f;

			float stretch = additionalStretch * 0.5f;

			for (int i = 0; i < filterWidth; i++)
			{
				float x = (i + offset) + nudge;

				double sincValue = sinc(x * stretch * extra1);
				double windowValue = sinc((x / halfWidth) * stretch * extra2);

				filter[i] = sincValue * windowValue;
			}
		}

		void getFilter(MipMapFilter mmFilter, std::vector<float>& filter)
		{
			const double kaiserAlpha = 4.0;
			const double kaiserStretch = 0.5;
			const int filterWidth = 14;

			switch (mmFilter)
			{
			case MMF_Point:
				filter = { 1, 0 };
				break;
			case MMF_Box:
				filter = { 1, 1 };
				break;
			case MMF_Gaussian:
				filter = { 1, 2, 1 };
				break;
			case MMF_Kaiser:
				getKaiserFilter(filterWidth, kaiserAlpha, kaiserStretch, filter);
				break;
			case MMF_Lanczos:
				getLanczosFilter(filterWidth, 1.0, 1.0, 1.0, filter);
				break;
			}

			normalizeFilter(filter);
		}

		void generateMipMaps(MipMapFilter mmFilter, int maxLevel)
		{
			std::vector<float> filter;

			getFilter(mmFilter, filter);

			generateMipMaps(filter, maxLevel);
		}

		template <bool yDir, int (*boundFunc)(int, int, int)>
		static glm::vec4 pixelGather(
			const Buffer2D<glm::vec4>& buffer, 
			const std::vector<float>& filter,
			const glm::ivec2& pos,
			const glm::ivec2& bufferBounds,
			int filterOffset)
		{
			glm::vec4 val(0.0f);

			for (int k = 0; k < filter.size(); k++)
			{
				glm::ivec2 samplePos = pos;

				if (yDir)
				{
					samplePos.x = pos.x;
					samplePos.y = boundFunc(pos.y + k + filterOffset, bufferBounds[0], bufferBounds[1]);
				}
				else
				{
					samplePos.x = boundFunc(pos.x + k + filterOffset, bufferBounds[0], bufferBounds[1]);
					samplePos.y = pos.y;
				}

				val += filter[k] * buffer(samplePos.x, samplePos.y);
			}

			return val;
		}

		static void downsample(
			const std::vector<float>& filter,
			const Buffer2D<glm::vec4>& original,
			Buffer2D<glm::vec4>& output,
			Buffer2D<glm::vec4>& temp)
		{
			output.resize(original.sizeX() / 2, original.sizeY() / 2);

			int filterOffset = -(int)(filter.size() / 2 - 1);

			constexpr auto boundaryFunc = &Boundary::reflective<int>;

			for (int y = 0; y < original.sizeY(); y++)
			{
				for (int x = 0; x < original.sizeX(); x+=2)
				{
					temp(x / 2, y) = pixelGather<false, boundaryFunc>(
						original, filter, glm::ivec2(x, y), glm::ivec2(0, original.sizeX() - 1), filterOffset);
				}
			}

			for (int y = 0; y < original.sizeY(); y+=2)
			{
				for (int x = 0; x < original.sizeX() / 2; x++)
				{
					output(x, y / 2) = pixelGather<true, boundaryFunc>(
						temp, filter, glm::ivec2(x, y), glm::ivec2(0, original.sizeY() - 1), filterOffset);
				}
			}
		}

		static void normalizeFilter(std::vector<float>& filter)
		{
			float sum = 0.0f;
			for (int i = 0; i < filter.size(); i++)
			{
				sum += filter[i];
			}

			for (int i = 0; i < filter.size(); i++)
			{
				filter[i] /= sum;
			}
		}

		void generateMipMaps(const std::vector<float>& filter, int maxLevel)
		{
			int filterWidth = filter.size();

			Buffer2D<glm::vec4> texBuffer[2];
			Buffer2D<glm::vec4> tempBuffer(dim.x, dim.y);

			int numMipMaps = glm::clamp(maxLevel, 0, maxPotentialLod());
			mipMaps.resize(numMipMaps);

			auto currentOriginalBuffer = &texBuffer[0];
			auto currentMipMapBuffer = &texBuffer[1];

			toVecBuffer(*currentOriginalBuffer);

			for (int i = 0; i < mipMaps.size(); i++)
			{
				downsample(filter, *currentOriginalBuffer, *currentMipMapBuffer, tempBuffer);
				mipMaps[i].fromVecBuffer(*currentMipMapBuffer);

				std::swap(currentOriginalBuffer, currentMipMapBuffer);
			}
		}

		/*void generateMipMaps(int maxLevel)
		{
			int numMipMaps = glm::clamp(maxLevel, 0, maxPotentialLod());
			mipMaps.resize(numMipMaps);

			for (int i = 0; i < mipMaps.size(); i++)
			{
				Texture2DCpu& mipMap = mipMaps[i];

				int mipMapDivide = std::pow(2, i + 1);

				mipMap.resize(dim.x / mipMapDivide, dim.y / mipMapDivide);

				for (int y = 0; y < mipMap.size().y; y++)
				{
					for (int x = 0; x < mipMap.size().x; x++)
					{
						int filterWidth = mipMapDivide;
						glm::dvec4 val;

						for (int xf = 0; xf < filterWidth; xf++)
						{
							for (int yf = 0; yf < filterWidth; yf++)
							{
								val += getPixelRaw(x * mipMapDivide + xf, y * mipMapDivide + yf);
							}
						}

						val /= (double)(filterWidth * filterWidth);

						mipMap.setPixelRaw(x, y, glm::round(val));
					}
				}
			}
		}*/

	public:
		TextureMip2DCpu() {}

		virtual void load(const std::string& filename)
		{
			Texture2DCpu::load(filename);

			generateMipMaps();
		}

		int maxLod() const
		{
			return mipMaps.size();
		}

		const Texture2DCpu& getMipMap(int lod) const
		{
			int lodClamp = glm::clamp(lod, 0, maxLod());

			if (lodClamp == 0)
				return *this;
			else
				return mipMaps[lodClamp - 1];
		}

		glm::vec4 sample(const vec2& pos, int lod) const
		{
			return getMipMap(lod).sample(pos);
		}

		virtual glm::vec4 sample(const vec2& pos, T lod) const
		{
			if (filterMode == FM_Nearest)
				return sample(pos, (int)std::round(lod));
			else if (filterMode == FM_Linear)
			{
				int lod0 = std::floor(lod);
				int lod1 = lod0 + 1;

				T t = lod - lod0;

				return glm::vec4((1 - t) * vec4(sample(pos, lod0)) + t * vec4(sample(pos, lod1)));
			}
		}

		virtual void saveMipMaps(const std::string& filename)
		{
			const std::string ending = ".png";
			std::string filepart;

			if (filename.size() >= ending.size() && std::equal(ending.rbegin(), ending.rend(), filename.rbegin()))
			{
				filepart = filename.substr(0, filename.length() - ending.length());
			}
			else
				filepart = filename;

			save(filepart + std::to_string(0) + ending);

			for (int i = 0; i < mipMaps.size(); i++)
			{
				mipMaps[i].save(filepart + std::to_string(i + 1) + ending);
			}
		}
	};
}