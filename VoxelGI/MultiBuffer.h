#pragma once

#include <vector>
#include <algorithm>

namespace VoxelGI
{
	class IBuffer3D
	{
	public:
		virtual ~IBuffer3D() {}

		virtual int sizeX() const = 0;
		virtual int sizeY() const = 0;
		virtual int sizeZ() const = 0;

		virtual void resize(int siX, int siY, int siZ) = 0;

		//virtual T& get(int x, int y, int z) = 0;
	};

	template <typename T>
	class Buffer3D : public IBuffer3D
	{
	private:
		T* buffer = nullptr;
		int sX = 0;
		int sY = 0;
		int sZ = 0;

		int bufferIndex(int x, int y, int z) const
		{
			return z * sY * sX + y * sX + x;
		}

		void release()
		{
			if (buffer != nullptr)
			{
				delete[] buffer;
				buffer = nullptr;
			}
		}

		template <typename T>
		static T clamp(const T& n, const T& lower, const T& upper) 
		{
			return std::max(lower, std::min(n, upper));
		}

	public:
		Buffer3D() {}
		Buffer3D(int siX, int siY, int siZ) { resize(siX, siY, siZ); }

		~Buffer3D() { release(); }

		T* data()
		{
			return buffer;
		}

		int sizeX() const
		{
			return sX;
		}

		int sizeY() const
		{
			return sY;
		}

		int sizeZ() const
		{
			return sZ;
		}

		int size() const
		{
			return sX * sY * sZ;
		}

		void resize(int siX, int siY, int siZ)
		{
			if (sX != siX || sY != siY || sZ != siZ)
			{
				release();

				sX = siX;
				sY = siY;
				sZ = siZ;

				buffer = new T[size()];
			}
		}

		void fill(T val)
		{
			std::fill(buffer, buffer + size(), val);
		}

		T& at(int x, int y, int z)
		{
			x = clamp(x, 0, sX);
			y = clamp(y, 0, sY);
			z = clamp(z, 0, sZ);
		}

		T& get(int x, int y, int z)
		{
			return buffer[bufferIndex(x, y, z)];
		}

		T& operator () (int x, int y, int z)
		{
			return buffer[bufferIndex(x, y, z)];
		}

		void copy(Buffer3D& outBuffer) const
		{
			outBuffer.resize(sX, sY, sZ);

			memcpy(outBuffer.buffer, buffer, size() * sizeof(T));
		}

		Buffer3D& operator=(const Buffer3D&) = delete;  // Disallow copying
		Buffer3D(const Buffer3D&) = delete;
	};

	/*template <typename T, int attr, typename B>
	class Buffer3DView : public IBuffer3D<T>
	{
	private:
		std::shared_ptr<Buffer3D<B>> data;

	public:
		Buffer3DView(const std::shared_ptr<Buffer3D<B>>& buf) : data(buf) {}

		virtual int sizeX() const
		{
			return data->sizeX();
		}

		virtual int sizeY() const
		{
			return data->sizeY();
		}

		virtual int sizeZ() const
		{
			return data->sizeZ();
		}

		virtual T& get(int x, int y, int z)
		{
			return data->get(x, y, z).get<VA_SignedDistance, T>();
			//return data->get(x, y, z).get<attr>();
		}
	};*/

	template <typename T>
	class Buffer2D
	{
	private:
		T* data = nullptr;
		int sX = 0;
		int sY = 0;

		int bufferIndex(int x, int y) const
		{
			return y * sX + x;
		}

		void release()
		{
			if (data != nullptr)
			{
				delete[] data;
				data = nullptr;
			}
		}

	public:
		enum DataComponent
		{
			DC_X,
			DC_Y,
			DC_Z,
			DC_Magnitude
		};

		Buffer2D() : data(nullptr) {}
		Buffer2D(int siX, int siY) { resize(siX, siY); }

		~Buffer2D() { release(); }

		Buffer2D& operator=(const Buffer2D& other)
		{
			if (&other == this)
				return *this;

			if (size != other.size)
			{
				resize(other.sX, other.sY);
			}

			//std::copy(&other.data[0], &other.data[0] + size, &data[0]);

			return *this;
		}

		int sizeX() const
		{
			return sX;
		}

		int sizeY() const
		{
			return sY;
		}

		int size() const
		{
			return sX * sY;
		}

		void clear(const T& val)
		{
			int s = size();

			for (int i = 0; i < s; i++)
				data[i] = val;
		}

		void resize(int siX, int siY)
		{
			if (sX != siX || sY != siY)
			{
				release();

				sX = siX;
				sY = siY;

				data = new T[size()];
			}
		}

		void fill(T val)
		{
			std::fill(data, data + size(), val);
		}

		/*template <std::size_t N>
		void fillValues(const std::array<T, N>& values)
		{
			int s = size();

			for (int i = 0; i < s; i++)
			{
				data[i] = values[i % values.size()];
			}
		}*/

		template <typename S>
		void extractValues(DataComponent comp, int index, bool yAxis, std::vector<S>& valuesOut) const
		{
			int length = yAxis ? sizeY() : sizeX();

			valuesOut.resize(length);

			for (int i = 0; i < length; i++)
			{
				int x, y;

				if (yAxis)
				{
					x = index;
					y = i;
				}
				else
				{
					x = i;
					y = index;
				}

				switch (comp)
				{
				case DC_X:
					valuesOut[i] = get(x, y)[0];
					break;
				case DC_Y:
					valuesOut[i] = get(x, y)[1];
					break;
				case DC_Z:
					valuesOut[i] = get(x, y)[2];
					break;
				case DC_Magnitude:
					valuesOut[i] = std::sqrt(get(x, y)[0] * get(x, y)[0] + get(x, y)[1] * get(x, y)[1] + get(x, y)[2] * get(x, y)[2]);
					break;
				}
			}
		}

		T& get(int x, int y)
		{
			return data[bufferIndex(x, y)];
		}

		T& operator () (int x, int y)
		{
			return data[bufferIndex(x, y)];
		}

		const T& get(int x, int y) const
		{
			return data[bufferIndex(x, y)];
		}

		const T& operator () (int x, int y) const
		{
			return data[bufferIndex(x, y)];
		}
	};
}