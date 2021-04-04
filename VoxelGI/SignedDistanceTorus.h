#pragma once

#include "SignedDistanceGeometry.h"

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceTorus : public SignedDistanceGeometry<T>
	{
	private:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;


		glm::vec2 radius = glm::vec2(0.7f, 0.25f);
		bool useTorusShapeNormal = false;

	public:
		SignedDistanceTorus() {}
		SignedDistanceTorus(const glm::vec2& r) : radius(r) {}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceTorus<T>& s = (SignedDistanceTorus<T>&)geoOut;

			s = *this;
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(-1.0, -1.0);
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			vec2 q = glm::vec2(glm::length(glm::vec2(pos.x, pos.z)) - radius.x, pos.y);

			return glm::length(q) - radius.y;
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			if (useTorusShapeNormal)
			{
				Float alpha = radius.x / glm::length(vec2(pos.x, pos.z));

				vec3 n = (pos * vec3(1 - alpha, 1, 1 - alpha)) / (T)radius.y;

				return n;
			}
			else
			{
				vec3 grad;

				Float lengthXZ = glm::length(glm::vec2(pos.x, pos.z));
				Float lengthXZRY = glm::length(glm::vec2(lengthXZ - radius.x, pos.y));
				Float lengthProdInv = (T)1.0 / (lengthXZRY * lengthXZ);

				grad.x = pos.x * ((lengthXZ - radius.x)) * lengthProdInv;
				grad.z = pos.z * ((lengthXZ - radius.x)) * lengthProdInv;

				grad.y = pos.y / lengthXZRY;

				return glm::normalize(grad);
			}
		}

		void calcTorusShapeNormalDiff(const vec3& pos, const vec3& dpdx, Float alpha, vec3& dndx) const
		{
			if (useTorusShapeNormal)
			{
				Float alphaDx = radius.x * -std::pow(glm::dot(vec2(pos.x, pos.z), vec2(pos.x, pos.z)), (T)-1.5)
					* (pos.x * dpdx.x + pos.z * dpdx.z);

				dndx = (dpdx * vec3(1 - alpha, 1, 1 - alpha) + pos * vec3(-alphaDx, 0, -alphaDx)) / (T)radius.y;
			}
		}

		void calcNormalDiff(const vec3& pos, const vec3& dpdx, Float lengthXZ, Float lengthXZRY, Float lengthProdInv, vec3& dndx) const
		{
			Float lengthXZDx = ((T)1 / glm::length(glm::vec2(pos.x, pos.z)))
				* (pos.x * dpdx.x + pos.z * dpdx.z);

			Float lengthXZRYDx = ((T)1 / (glm::length(glm::vec2(lengthXZ - radius.x, pos.y))))
				* ((lengthXZ - radius.x) * lengthXZDx + pos.y * dpdx.y);

			Float lengthProdInvDx = ((T)-1.0 / (lengthXZRY * lengthXZ * lengthXZRY * lengthXZ))
				* (lengthXZRYDx * lengthXZ + lengthXZRY * lengthXZDx);

			dndx.x = dpdx.x * (lengthXZ - radius.x) * lengthProdInv
				+ pos.x * lengthXZDx * lengthProdInv
				+ pos.x * (lengthXZ - radius.x) * lengthProdInvDx;

			dndx.z = dpdx.z * (lengthXZ - radius.x) * lengthProdInv
				+ pos.z * lengthXZDx * lengthProdInv
				+ pos.z * (lengthXZ - radius.x) * lengthProdInvDx;

			dndx.y = (lengthXZRY * dpdx.y - pos.y * lengthXZRYDx) / (lengthXZRY * lengthXZRY);
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			IGeometry<T>::NormalDifferential& dn) const
		{
			if (useTorusShapeNormal)
			{
				vec3 pos = projectPosToObject(rayDiff.getPosition());
				vec3 dpdx = projectPosDiffToObject(rayDiff.getDpdx());
				vec3 dpdy = projectPosDiffToObject(rayDiff.getDpdy());

				Float alpha = radius.x / glm::length(vec2(pos.x, pos.z));

				dn.n = (pos * vec3(1 - alpha, 1, 1 - alpha)) / (T)radius.y;
				
				calcTorusShapeNormalDiff(pos, dpdx, alpha, dn.dndx);
				calcTorusShapeNormalDiff(pos, dpdy, alpha, dn.dndy);

				dn.computeDN(true);
			}
			else
			{
				vec3 pos = projectPosToObject(rayDiff.getPosition());
				vec3 dpdx = projectPosDiffToObject(rayDiff.getDpdx());
				vec3 dpdy = projectPosDiffToObject(rayDiff.getDpdy());

				Float lengthXZ = glm::length(glm::vec2(pos.x, pos.z));
				Float lengthXZRY = glm::length(glm::vec2(lengthXZ - radius.x, pos.y));
				Float lengthProdInv = (T)1.0 / (lengthXZRY * lengthXZ);

				vec3 grad;
				grad.x = pos.x * (lengthXZ - radius.x) * lengthProdInv;
				grad.z = pos.z * (lengthXZ - radius.x) * lengthProdInv;
				grad.y = pos.y / lengthXZRY;

				dn.n = grad;

				// Diff
				calcNormalDiff(pos, dpdx, lengthXZ, lengthXZRY, lengthProdInv, dn.dndx);
				calcNormalDiff(pos, dpdy, lengthXZ, lengthXZRY, lengthProdInv, dn.dndy);

				dn.computeDN(true);
			}
		}

		virtual SignedDistanceGeometry<T>::SdfType getSdfType() const
		{
			return ST_Torus;
		}

		virtual void drawSdfDebugGui() override
		{
			Gui::Property("Radius", radius);
			Gui::Property("Use Torus Shape Normal", useTorusShapeNormal);
		}
	};
}