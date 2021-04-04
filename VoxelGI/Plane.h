#pragma once

#include <glm\glm.hpp>

namespace VoxelGI
{
	template <typename T>
	class Plane
	{
		using vec3 = glm::tvec3<T>;

	public:
		vec3 point;
		vec3 normal;

		Plane() {}
		Plane(const vec3& p, const vec3& n) : point(p), normal(n) {}
	};
}