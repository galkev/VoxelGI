#pragma once

#include "SignedDistanceGeometry.h"

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceSphere : public SignedDistanceGeometry<T>
	{
	private:
		float radius;

	public:
		SignedDistanceSphere(float r = 1.0f) : radius(r) {}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceSphere<T>& s = (SignedDistanceSphere<T>&)geoOut;

			s = *this;
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			return glm::length(pos) - radius;
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			return pos / glm::length(pos);
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(-1.0, -1.0);
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			NormalDifferential& dn) const
		{
			// project to object rel
			vec3 P = projectPosToObject(rayDiff.getPosition());
			vec3 dPdx = projectPosDiffToObject(vec3(rayDiff.getDpdx()));
			vec3 dPdy = projectPosDiffToObject(vec3(rayDiff.getDpdy()));

			dn.n = vec4(getGradient(P), 0.0);
			dn.dndx = vec4((glm::dot(P, P) * dPdx - glm::dot(dPdx, P) * P) / pow(glm::dot(P, P), (Float)1.5), 0.0);
			dn.dndy = vec4((glm::dot(P, P) * dPdy - glm::dot(dPdy, P) * P) / pow(glm::dot(P, P), (Float)1.5), 0.0);

			dn.computeDN(true);
		}

		virtual SdfType getSdfType() const
		{
			return ST_Sphere;
		}

		virtual void drawSdfDebugGui() override
		{
			Gui::Property("Radius", radius);
		}
	};
}