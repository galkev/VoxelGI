#pragma once

#include "Geometry.h"
#include "Gui.h"

namespace VoxelGI
{
	template <typename T>
	class Sphere : public Geometry<T>
	{
	private:
		glm::vec3 center;
		float radius;

		bool useSphereDt = false;

		vec2 getUVDx(const vec3& pos, const vec3& dpdx) const
		{
			/*
			vec3 n = surfaceNormal(pos);
			
			return vec2(
				std::atan2(n.x, n.z) / (2*glm::pi<T>()) + 0.5,
				n.y * 0.5 + 0.5
			); 
			*/

			vec3 n = surfaceNormal(pos);
			vec3 dndx = dpdx / (Float)radius;

			vec2 atan2dn = vec2(n.z, -n.x) / (n.x * n.x + n.z * n.z);

			return vec2(
				(atan2dn[0] * dndx.x + atan2dn[1] * dndx.z) / (2 * glm::pi<T>()),
				dndx.y * 0.5
			);
		}

	public:
		Sphere() {}
		Sphere(const glm::vec3& c, float r) : center(c), radius(r) {}

		virtual vec3 surfaceNormal(const vec3& pos) const
		{
			return (pos - vec3(center)) / (Float)radius;
			//return glm::normalize(pos - center);
		}

		virtual vec3 shadingNormal(const vec3& pos) const
		{
			return surfaceNormal(pos);
		}

		virtual glm::vec4 surfaceColor(const RayDifferential<Float>& rayDiff) const
		{
			return glm::vec4(0.0);
		}
		
		virtual void copy(IGeometry<T>& geoOut) const
		{
			Sphere<T>& s = (Sphere<T>&)geoOut;

			s = *this;
		}

		virtual void transform(const glm::mat4& transformMat)
		{
			center = vec3(transformMat * vec4(center, 1.0));
			radius = radius;
		}

		// Differentials
		/*virtual void getDt(const RayDifferential<Float>& rayDiff, Float& dtdx, Float& dtdy) const override
		{
			if (useSphereDt)
			{
				vec3 cp = rayDiff.origin - vec3(center);
				Float b = glm::dot(cp, rayDiff.direction);
				Float c = glm::dot(cp, cp) - radius * radius;

				Float dbdx = glm::dot(rayDiff.getDpdx(), rayDiff.direction) + glm::dot(cp, rayDiff.getDddx());
				Float dbdy = glm::dot(rayDiff.getDpdy(), rayDiff.direction) + glm::dot(cp, rayDiff.getDddy());

				Float dcdx = 2 * glm::dot(rayDiff.getDpdx(), cp);
				Float dcdy = 2 * glm::dot(rayDiff.getDpdy(), cp);

				dtdx = -dbdx - (2 * b * dbdx - dcdx) / (2 * std::sqrt(b*b - c));
				dtdy = -dbdy - (2 * b * dbdy - dcdy) / (2 * std::sqrt(b*b - c));
			}
			else
			{
				Geometry::getDt(rayDiff, dtdx, dtdy);
			}
		}*/

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			NormalDifferential& dn) const
		{
			dn.n = shadingNormal(rayDiff.getPosition());
			dn.dndx = rayDiff.getDpdx() / (Float)radius;
			dn.dndy = rayDiff.getDpdy() / (Float)radius;

			dn.computeDN(false);
		}

		virtual void getDuv(
			const RayDifferential<Float>& rayDiff,
			vec2& duvdx,
			vec2& duvdy) const
		{
			duvdx = getUVDx(rayDiff.getPosition(), rayDiff.getDpdx());
			duvdy = getUVDx(rayDiff.getPosition(), rayDiff.getDpdy());
		}

		virtual vec2 getUV(const vec3& pos) const
		{ 
			vec3 n = surfaceNormal(pos);
			
			return vec2(
				std::atan2(n.x, n.z) / (2*glm::pi<T>()) + 0.5,
				n.y * 0.5 + 0.5
			); 
		}

		virtual bool transfer(const Ray<T>& ray) const
		{
			vec3 m = ray.origin - vec3(center);
			Float b = glm::dot(m, ray.direction);
			Float c = glm::dot(m, m) - radius * radius;

			// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
			if (c > 0.0f && b > 0.0f) 
				return false;
			
			Float discr = b * b - c;

			// A negative discriminant corresponds to ray missing sphere 
			if (discr < 0.0f) 
				return false;

			// Ray now found to intersect sphere, compute smallest t value of intersection
			ray.t = -b - std::sqrt(discr);

			// If t is negative, ray started inside sphere so clamp t to zero 
			if (ray.t < 0.0f) 
				ray.t = 0.0f;

			return true;
		}

		virtual GeometryType geoType() const
		{
			return GT_Sphere;
		}

		float getRadius()
		{
			return radius;
		}

		void setRadius(float r)
		{
			radius = r;
		}

		virtual void drawDebugGui() override
		{
			Gui::Property("Radius", radius);
			Gui::Property("Sphere Dt", useSphereDt);
		}
	};
}