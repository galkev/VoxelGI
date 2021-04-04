#include "pbrtDifferentialsHelper.h"

#include "interaction.h"

#include "triangle.h"

namespace VoxelGI
{

	pbrtDifferentialsHelper::pbrtDifferentialsHelper()
	{
	}


	pbrtDifferentialsHelper::~pbrtDifferentialsHelper()
	{
	}

	pbrt::RayDifferential pbrtDifferentialsHelper::convertRay(const RayDifferential & ray)
	{
		pbrt::RayDifferential pbrtRay(ray.origin, ray.direction);

		pbrtRay.rxOrigin = ray.rx.origin;
		pbrtRay.rxDirection = ray.rx.direction;
		pbrtRay.ryOrigin = ray.ry.origin;
		pbrtRay.ryDirection = ray.ry.direction;
		pbrtRay.hasDifferentials = true;

		return pbrtRay;
	}

	void VoxelGI::pbrtDifferentialsHelper::getDifferentials(const RayDifferential& ray, const glm::vec3& surfacePoint, const glm::vec3& surfaceNormal, glm::vec3& dpdx, glm::vec3& dpdy)
	{
		glm::vec3 dpdu;
		glm::vec3 dpdv;
		glm::vec3 dndu;
		glm::vec3 dndv;

		pbrt::SurfaceInteraction surfaceInteraction(surfacePoint, glm::vec3(), glm::vec2(), glm::vec3(), dpdu, dpdv, dndu, dndv, 0.0f, nullptr, 0);
		surfaceInteraction.n = surfaceNormal;

		pbrt::RayDifferential pbrtRay(ray.origin, ray.direction);

		pbrtRay.rxOrigin = ray.rx.origin;
		pbrtRay.rxDirection = ray.rx.direction;
		pbrtRay.ryOrigin = ray.ry.origin;
		pbrtRay.ryDirection = ray.ry.direction;
		pbrtRay.hasDifferentials = true;

		surfaceInteraction.ComputeDifferentials(pbrtRay);

		dpdx = surfaceInteraction.dpdx;
		dpdy = surfaceInteraction.dpdy;
	}

	bool pbrtDifferentialsHelper::triangleTest(const RayDifferential& ray, Triangle<Vertex>& triangle, glm::vec3 & dpdx, glm::vec3 & dpdy)
	{
		glm::vec3 vertices[] = { triangle.point(0).pos, triangle.point(1).pos, triangle.point(2).pos };
		glm::vec3 normals[] = { triangle.point(0).normal, triangle.point(1).normal, triangle.point(2).normal };
		int indices[] = { 0, 1, 2 };

		auto pbrtTriMesh = std::make_shared<pbrt::TriangleMesh>(indices[0], 1, indices, 3, vertices, nullptr, normals, nullptr, nullptr);

		pbrt::Triangle pbrtTri(nullptr, nullptr, false, pbrtTriMesh, 0);

		float tHit;
		pbrt::SurfaceInteraction interaction;
		
		bool hit = pbrtTri.Intersect(convertRay(ray), &tHit, &interaction, false);

		if (hit)
		{
			interaction.ComputeDifferentials(convertRay(ray));

			dpdx = interaction.dpdx;
			dpdy = interaction.dpdy;
		}

		return hit;
	}
}