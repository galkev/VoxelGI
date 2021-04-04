#pragma once

#include "../Ray.h"
#include "../Triangle.h"
#include "../Vertex.h"
#include "geometry.h"

namespace VoxelGI
{
	class pbrtDifferentialsHelper
	{
	public:
		pbrtDifferentialsHelper();
		~pbrtDifferentialsHelper();

		static pbrt::RayDifferential convertRay(const RayDifferential& ray);

		static void getDifferentials(const RayDifferential& ray, const glm::vec3& surfacePoint, const glm::vec3& surfaceNormal, glm::vec3& dpdx, glm::vec3& dpdy);
		static bool triangleTest(const RayDifferential& ray, Triangle<Vertex>& triangle, glm::vec3& dpdx, glm::vec3& dpdy);
	};
}
