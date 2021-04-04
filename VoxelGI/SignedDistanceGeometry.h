#pragma once

#include "Geometry.h"
#include "Gui.h"

namespace VoxelGI
{
	template<typename T>
	class SignedDistanceGeometry : public Geometry<T>
	{
	protected:
		glm::vec3 origin = glm::vec3(0.0f);
		float scale = 1.0f;

		bool boundIntersectOnly = false;

		static int maxRaySteps;
		static T sdfEps;

		vec3 lowerBound() const
		{
			return origin - scale;
		}

		vec3 upperBound() const
		{
			return origin + scale;
		}

		vec3 projectPosToObject(const vec3& globalPos) const
		{
			return (globalPos - vec3(origin)) / (Float)scale;
		}

		vec3 projectPosDiffToObject(const vec3& globalPosDiff) const
		{
			return globalPosDiff / (Float)scale;
		}

		vec3 projectPosToWorld(const vec3& localPos) const
		{
			//return (globalPos - vec3(origin)) / (Float)scale;
			return localPos * (Float)scale + vec3(origin);
		}

		bool rayBoxIntersect(const Ray<T>& ray, Float& tStart, Float& tEnd) const
		{
			vec3 boundLower = lowerBound();
			vec3 boundUpper = upperBound();

			Float tmin = (boundLower.x - ray.origin.x) / ray.direction.x;
			Float tmax = (boundUpper.x - ray.origin.x) / ray.direction.x;

			if (tmin > tmax) std::swap(tmin, tmax);

			Float tymin = (boundLower.y - ray.origin.y) / ray.direction.y;
			Float tymax = (boundUpper.y - ray.origin.y) / ray.direction.y;

			if (tymin > tymax) std::swap(tymin, tymax);

			if ((tmin > tymax) || (tymin > tmax))
				return false;

			if (tymin > tmin)
				tmin = tymin;

			if (tymax < tmax)
				tmax = tymax;

			Float tzmin = (boundLower.z - ray.origin.z) / ray.direction.z;
			Float tzmax = (boundUpper.z - ray.origin.z) / ray.direction.z;

			if (tzmin > tzmax) std::swap(tzmin, tzmax);

			if ((tmin > tzmax) || (tzmin > tmax))
				return false;

			if (tzmin > tmin)
				tmin = tzmin;

			if (tzmax < tmax)
				tmax = tzmax;

			if (tmax < 0.0)
				return false;

			tStart = tmin;
			tEnd = tmax;

			return true;
		}

	public:
		enum SdfType
		{
			ST_Sphere,
			ST_Ellipsoid,
			ST_Box,
			ST_Torus,
			ST_Triangle,
			ST_Voxel,
			ST_Bend,
			ST_Mesh
		};

		static int getMaxRaySteps()
		{
			return maxRaySteps;
		}

		static void setMaxRaySteps(int steps)
		{
			maxRaySteps = steps;
		}

		static T getSdfEps()
		{
			return sdfEps;
		}

		static void setSdfEps(T eps)
		{
			sdfEps = eps;
		}

		virtual void setScale(float s)
		{
			scale = s;
		}

		float getScale()
		{
			return scale;
		}

		virtual GeometryType geoType() const
		{
			return GT_Sdf;
		}

		bool isBoundIntersectOnly()
		{
			return boundIntersectOnly;
		}

		void setBoundIntersectOnly(bool boundOnly)
		{
			boundIntersectOnly = boundOnly;
		}

		virtual bool transfer(const Ray<T>& ray) const
		{
			//std::cout << ray.t << std::endl;

			Float tStart, tEnd;

			if (rayBoxIntersect(ray, tStart, tEnd) && ray.t < tEnd)
			{
				ray.t = std::max(tStart, (Float)ray.t);

				if (boundIntersectOnly)
					return true;

				for (int i = 0; i < maxRaySteps; i++)
				{
					Float sdf = getSignedDistance(projectPosToObject(ray.getPosition()));

					if (sdf <= sdfEps)
					{
						return true;
					}
					else
					{
						ray.t += sdf * (Float)scale;

						if (ray.t > tEnd)
							break;
					}
				}
			}

			return false;
		}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceGeometry<T>& s = (SignedDistanceGeometry<T>&)geoOut;

			s = *this;
		}

		virtual void transform(const glm::mat4& transformMat)
		{
			origin = vec3(transformMat * vec4(origin, 1.0));
			scale = scale;
		}

		virtual glm::vec4 surfaceColor(const RayDifferential<Float>& rayDiff) const
		{
			vec3 position = rayDiff.getPosition();

			return glm::vec4(getSignedDistance(rayDiff.origin - rayDiff.getDistance() * rayDiff.direction));
			//return glm::vec4(getGradient(projectPosToObject(position)), 1.0);
			//return glm::vec4(glm::vec3(getSignedDistance(projectPosToObject(position))), 1.0f);
			//return glm::vec4((projectToObjectRel(position) + vec3(1.0)) * (Float)0.5, (Float)1.0);
		}

		virtual vec3 surfaceNormal(const vec3& pos) const
		{
			return glm::normalize(getGradient(projectPosToObject(pos)));
		}

		virtual vec3 shadingNormal(const vec3& pos) const
		{
			return glm::normalize(surfaceNormal(pos));
		}

		virtual void drawDebugGui() override
		{
			Gui::Property("Max Ray Steps", maxRaySteps);
			Gui::Property("SDF Eps", sdfEps);
			Gui::Property("Bound Intersect Only", boundIntersectOnly);

			drawSdfDebugGui();
		}

		virtual T getSignedDistance(const vec3& pos) const = 0;
		virtual vec3 getGradient(const vec3& pos) const = 0;
		virtual SdfType getSdfType() const = 0;
		virtual void drawSdfDebugGui() {}
	};

	template <typename T>
	int SignedDistanceGeometry<T>::maxRaySteps = 100;

	template <typename T>
	T SignedDistanceGeometry<T>::sdfEps = 0.001;
}