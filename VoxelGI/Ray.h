#pragma once

#include <glm\glm.hpp>

#include "Plane.h"
#include "Vertex.h"
//#include "Camera.h"
//#include "Geometry.h"

class Camera;

template <typename T>
class RayTracerCpu;

namespace VoxelGI
{
	template <typename T>
	class IGeometry;

	template <typename T>
	class Ray
	{
	protected:
		using Float = T;
		using vec3 = glm::tvec3<T>;
		using vec4 = glm::tvec4<T>;

		virtual void initViewRayUnnormalized(
			const Camera& camera,
			Float x,
			Float y)
		{
			origin = camera.translation();
			direction = vec3(camera.forward()) + x * vec3(camera.right()) + y * vec3(camera.up());
		}

		virtual void reflect(const vec3& normal)
		{
			direction = direction - 2 * glm::dot(direction, normal) * normal;
			reflectCount++;
			t = 0.01;
		}

		Float currentDistance = 0.0;

	public:
		Ray() {}
		~Ray() {}

		vec3 origin;
		vec3 direction;
		int reflectCount = 0;

		mutable T t = 0.0;

		virtual void normalize()
		{
			direction = glm::normalize(direction);
		}

		vec3 getPosition() const
		{
			return origin + t * direction;
		}

		Float getDistance() const
		{
			return currentDistance + t;
		}

		virtual void transfer(const IGeometry<T>* geometry, bool calcDifferentials = true)
		{
			origin = getPosition();
			currentDistance += t;
			t = 0.0;
		}

		virtual void reflect(const IGeometry<T>* geometry)
		{
			reflect(geometry->surfaceNormal(getPosition()));
		}

		virtual void initViewRay(
			const Camera& camera,
			Float x,
			Float y)
		{
			initViewRayUnnormalized(camera, x, y);
			normalize();
		}
	};

	template <typename T>
	class RayDifferential : public Ray<T>
	{
	private:
		mutable vec3 dpdx;
		mutable vec3 dpdy;

		mutable vec3 dddx;
		mutable vec3 dddy;

	public:
		RayDifferential() {}
		RayDifferential(const Ray<T>& ray) : Ray(ray) {}

		virtual void initViewRay(
			const Camera& camera,
			Float x,
			Float y)
		{
			initViewRayUnnormalized(camera, x, y);

			vec3 dir = direction;
			vec3 up = camera.up();
			vec3 right = camera.right();

			dpdx = vec3(0.0);
			dpdy = vec3(0.0);

			dddx = (glm::dot(dir, dir) * right - glm::dot(dir, right) * dir)
				/ std::pow(glm::dot(dir, dir), (Float)1.5);

			dddy = (glm::dot(dir, dir) * up - glm::dot(dir, up) * dir)
				/ std::pow(glm::dot(dir, dir), (Float)1.5);

			normalize();
		}

		/*void initDifferentials(
			const Camera& camera,
			Float x,
			Float y)
		{
			vec3 dir = camera.createViewRayUnnormalized(x, y).direction;
			vec3 up = camera.up();
			vec3 right = camera.right();

			dpdx = vec3(0.0);
			dpdy = vec3(0.0);

			dddx = (glm::dot(dir, dir) * right - glm::dot(dir, right) * dir)
				/ std::pow(glm::dot(dir, dir), (Float)1.5);

			dddy = (glm::dot(dir, dir) * up - glm::dot(dir, up) * dir)
				/ std::pow(glm::dot(dir, dir), (Float)1.5);
		}*/

		/*void transfer(const Plane<T>& plane) const
		{
			Float dtdx, dtdy;

			dtdx = -glm::dot(dpdx + t * dddx, plane.normal) / glm::dot(direction, plane.normal);
			dtdy = -glm::dot(dpdy + t * dddy, plane.normal) / glm::dot(direction, plane.normal);

			dpdx = (dpdx + t * dddx) + dtdx * direction;
			dpdy = (dpdy + t * dddy) + dtdy * direction;

			//dddx = dddx;
			//dddy = dddy;
		}*/

		virtual void transfer(const IGeometry<T>* geometry, bool calcDifferentials) override
		{
			if (calcDifferentials)
			{
				Float dtdx, dtdy;

				geometry->getDt(*this, dtdx, dtdy);

				dpdx = (dpdx + t * dddx) + dtdx * direction;
				dpdy = (dpdy + t * dddy) + dtdy * direction;
			}

			//dddx = dddx;
			//dddy = dddy;

			Ray::transfer(geometry);
		}

		virtual void reflect(const IGeometry<T>* geometry)
		{
			IGeometry<T>::NormalDifferential dn;
			vec3 normal, dNdx, dNdy;

			geometry->getDn(*this, dn);

			normal = dn.N;
			dNdx = dn.dNdx;
			dNdy = dn.dNdy;

			//dpdx = dpdx;
			//dpdy = dpdy;

			T dDNdx = glm::dot(dddx, normal) + glm::dot(direction, dNdx);
			T dDNdy = glm::dot(dddy, normal) + glm::dot(direction, dNdy);

			dddx = dddx - (T)2.0 * (glm::dot(direction, normal) * dNdx + dDNdx * normal);
			dddy = dddy - (T)2.0 * (glm::dot(direction, normal) * dNdy + dDNdy * normal);

			Ray::reflect(normal);
		}

		vec3 getDpdx() const
		{
			return dpdx;
		}

		vec3 getDpdy() const
		{
			return dpdy;
		}

		vec3 getDddx() const
		{
			return dddx;
		}

		vec3 getDddy() const
		{
			return dddy;
		}
	};

	/*template <typename T>
	class RayDifferentialFinite : public Ray<T>
	{
	private:
		mutable vec4 dpdx;
		mutable vec4 dpdy;

	public:
		RayDifferentialFinite() {}
		RayDifferentialFinite(const Ray<T>& ray)
			: Ray(ray)
		{}

		Ray<T> rx;
		Ray<T> ry;

		Float dx, dy;

		void transfer(const Plane<T>& plane) const
		{
			RayTracerCpu<T>::rayPlaneIntersect(rx, plane);
			RayTracerCpu<T>::rayPlaneIntersect(ry, plane);

			dpdx = vec4(rx.getPosition() - getPosition(), 0.0f) / dx;
			dpdy = vec4(ry.getPosition() - getPosition(), 0.0f) / dy;
		}

		const vec4& getDpdx() const
		{
			return dpdx;
		}

		const vec4& getDpdy() const
		{
			return dpdy;
		}

		void initDifferentials(
			const Camera& camera,
			Float x,
			Float y,
			Float dX,
			Float dY)
		{
			dx = dX;
			dy = dY;
			rx = camera.createViewRay(x + dx, y);
			ry = camera.createViewRay(x, y + dy);
		}
	};*/
}