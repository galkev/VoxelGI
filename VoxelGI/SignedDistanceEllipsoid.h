#pragma once

#include "SignedDistanceGeometry.h"

#include <glm/gtx/component_wise.hpp>

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceEllipsoid : public SignedDistanceGeometry<T>
	{
	private:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;

		glm::vec3 radius;

	public:
		SignedDistanceEllipsoid() : radius(1.0) {}
		SignedDistanceEllipsoid(const glm::vec3& r) : radius(r) {}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceEllipsoid<T>& s = (SignedDistanceEllipsoid<T>&)geoOut;

			s = *this;
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(-1.0, -1.0);
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			return (glm::length(pos / vec3(radius)) - 1.0) *glm::compMin(radius);
			//return glm::length(pos) - radius;
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			return glm::normalize(((pos / vec3(radius * radius)) / glm::length(pos / vec3(radius))) * (T)glm::compMin(radius));
			//return pos / glm::length(pos);
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			IGeometry<T>::NormalDifferential& dn) const
		{
			// project to object rel
			/*vec3 P = projectPosToObject(rayDiff.getPosition());
			vec3 dPdx = projectPosDiffToObject(vec3(rayDiff.getDpdx()));
			vec3 dPdy = projectPosDiffToObject(vec3(rayDiff.getDpdy()));

			dn.n = vec4(getGradient(P), 0.0);
			dn.dndx = vec4((glm::dot(P, P) * dPdx - glm::dot(dPdx, P) * P) / pow(glm::dot(P, P), (Float)1.5), 0.0);
			dn.dndy = vec4((glm::dot(P, P) * dPdy - glm::dot(dPdy, P) * P) / pow(glm::dot(P, P), (Float)1.5), 0.0);

			dn.computeDN(true);*/
		}

		virtual SignedDistanceGeometry<T>::SdfType getSdfType() const
		{
			return ST_Ellipsoid;
		}

		virtual void drawSdfDebugGui() override
		{
			Gui::Property("Radius", radius);
		}
	};
}