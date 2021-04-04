#pragma once

#include "VoxelInterpolator.h"

#include <array>

namespace VoxelGI
{
	template <typename T>
	class VoxelInterpolatorNearest : public VoxelInterpolator<T>
	{
	public:
		virtual T filter(const vec3& pos) const
		{
			vec3 posBuffer = coordsRelToAbs(pos);

			glm::ivec3 bufferIndex = glm::clamp(glm::ivec3(glm::round(posBuffer - vec3(0.5))), glm::ivec3(0), getResolution() - glm::ivec3(1));

			return getData()->get(bufferIndex.x, bufferIndex.y, bufferIndex.z);
		}

		virtual vec3 gradient(const vec3& pos) const
		{
			return vec3(0);
		}

		virtual void gradientDifferential(
			const vec3& pos,
			const RayDifferential<Float>& rayDiff,
			typename IGeometry<T>::NormalDifferential& dn) const
		{

		}
	};
}