#pragma once

#include <glm\glm.hpp>
#include "Geometry.h"

namespace VoxelGI
{
	template <typename T>
	class IVoxelInterpolator
	{
	public:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;

		virtual glm::ivec3 getResolution() const = 0;

		virtual void setData(const std::shared_ptr<Buffer3D<T>>& buf) = 0;
		virtual const std::shared_ptr<Buffer3D<T>>& getData() const = 0;
		virtual std::shared_ptr<Buffer3D<T>>& getData() = 0;

		virtual void setScale(T s) = 0;

		// pos in [-1; 1]
		virtual T filter(const vec3& pos) const = 0;
		virtual vec3 gradient(const vec3& pos) const = 0;
		virtual void hessian(
			const vec3& pos,
			vec3& gradientVec,
			glm::tmat3x3<T>& hessianMat) const = 0;
	};

	template <typename T>
	class VoxelInterpolator : public IVoxelInterpolator<T>
	{
	private:
		std::shared_ptr<Buffer3D<T>> voxelData;

		template <int neighborPerAxis>
		void getCellIndices(
			const glm::ivec3& pos,
			const glm::ivec3& minPos,
			const glm::ivec3& maxPos,
			std::array<glm::ivec3, neighborPerAxis*neighborPerAxis*neighborPerAxis>& indices) const
		{
			glm::ivec3 n;
			glm::ivec3 startPos = pos - glm::ivec3(neighborPerAxis / 2 - 1);

			int i = 0;

			for (n.z = 0; n.z < neighborPerAxis; n.z++)
			{
				for (n.y = 0; n.y < neighborPerAxis; n.y++)
				{
					for (n.x = 0; n.x < neighborPerAxis; n.x++)
					{
						indices[i++] = glm::clamp(startPos + n, minPos, maxPos);
					}
				}
			}
		}

	protected:
		T scale = (T)1;

		template <int d, typename A>
		A& bufferAt(std::array<A, d*d*d> arr, int x, int y, int z) const
		{
			return arr[z * d * d + y * d + x];
		}

		vec3 coordsAbsToRel(const vec3& coordsAbs) const
		{
			return coordsAbs / vec3(getResolution()) * (T)2.0 - vec3(1);
		}

		vec3 coordsRelToAbs(const vec3& coordsRel) const
		{
			return (coordsRel + vec3(1)) * (T)0.5 * vec3(getResolution());
		}

		template <int neighborPerAxis>
		__forceinline vec3 getNeighborCells(const vec3& posBuffer, std::array<T, neighborPerAxis*neighborPerAxis*neighborPerAxis>& cells) const
		{
			constexpr int numNeighbors = neighborPerAxis* neighborPerAxis*neighborPerAxis;

			glm::ivec3 cell = glm::ivec3(glm::floor(posBuffer - vec3(0.5f)));

			std::array<glm::ivec3, numNeighbors> indices;
			getCellIndices<neighborPerAxis>(cell, glm::ivec3(0), getResolution() - glm::ivec3(1), indices);

			for (int i = 0; i < numNeighbors; i++)
				cells[i] = voxelData->get(indices[i].x, indices[i].y, indices[i].z);

			return posBuffer - (vec3(cell) + vec3(0.5));
		}

	public:
		virtual void setData(const std::shared_ptr<Buffer3D<T>>& buf)
		{
			voxelData = buf;
		}

		virtual const std::shared_ptr<Buffer3D<T>>& getData() const
		{
			return voxelData;
		}

		virtual std::shared_ptr<Buffer3D<T>>& getData()
		{
			return voxelData;
		}

		virtual void setScale(T s)
		{
			scale = s;
		}

		virtual glm::ivec3 getResolution() const
		{
			return glm::ivec3(voxelData->sizeX(), voxelData->sizeY(), voxelData->sizeZ());
		}
	};
}