#pragma once

#include "SignedDistanceGeometry.h"
#include <memory>

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceBend : public SignedDistanceGeometry<T>
	{
	private:
		float angle;
		std::shared_ptr<SignedDistanceGeometry> sdfGeo;

	public:
		SignedDistanceBend() {}
		SignedDistanceBend(const std::shared_ptr<SignedDistanceGeometry>& sdf, float bendAngle = 90.0f) : sdfGeo(sdf), angle(bendAngle) {}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceBend<T>& s = (SignedDistanceBend<T>&)geoOut;

			s = *this;
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(-1.0, -1.0);
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			/*T c = std::cos(glm::radians(20.0*pos.y));
			T s = std::sin(glm::radians(20.0*pos.y));
			glm::tmat2x2<T> m(c, -s, s, c);

			vec2 bendPos = m * vec2(pos);

			vec3  q = vec3(bendPos[0], bendPos[1], pos.z);*/

			T c = std::cos(glm::radians((T)angle*pos.x));
			T s = std::sin(glm::radians((T)angle*pos.x));
			glm::tmat2x2<T> m(c, -s, s, c);

			vec2 bendPos = m * vec2(pos.x, pos.z);

			vec3  q = vec3(bendPos[0], pos.y, bendPos[1]);

			return sdfGeo->getSignedDistance(q);
			//return sdfGeo->getSignedDistance(q);
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			return vec3();
			//return pos / glm::length(pos);
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			NormalDifferential& dn) const
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

		virtual SdfType getSdfType() const
		{
			return ST_Bend;
		}

		virtual void drawSdfDebugGui() override
		{
			Gui::Property("Angle", angle);
			Gui::Text("Geometry");
			sdfGeo->drawDebugGui();
		}
	};
}