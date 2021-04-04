#pragma once

#include "SignedDistanceGeometry.h"

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceBox : public SignedDistanceGeometry<T>
	{
	private:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;

		float r;
		glm::vec3 extent = glm::vec3(0.5f);

	public:
		SignedDistanceBox() {}
		SignedDistanceBox(const glm::vec3& ext, float rnd = 0.0f) : extent(ext), r(rnd) {}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceBox<T>& s = (SignedDistanceBox<T>&)geoOut;

			s = *this;
		}

		T lengthn(const vec3& v, int n) const
		{
			return std::pow(std::pow(v.x, n) + std::pow(v.y, n) + std::pow(v.z, n), 1.0 / n);
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			vec3 d = glm::abs(pos) - vec3(extent);

			return std::min(std::max(d.x, std::max(d.y, d.z)), (T)0.0) + glm::length(glm::max(d, vec3(0.0))) - r;
			//return std::min(std::max(d.x, std::max(d.y, d.z)), (T)0.0) + lengthn(glm::max(d, vec3(0.0)), 1);
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			return vec3();

			/*vec3 grad;

			int maxIndex;

			if (pos.x > pos.y)
			{
				if (pos.x > pos.z)
					maxIndex = 0;
				else
					maxIndex = 2;
			}
			else
			{
				if (pos.y > pos.z)
					maxIndex = 1;
				else
					maxIndex = 2;
			}

			Float l = glm::length(glm::max(pos, vec3(0)));

			if (l == (Float)0)
				l = (Float)1;

			for (int i = 0; i < 3; i++)
			{
				if (pos[i] < (Float)0)
				{
					if (i == maxIndex)
						grad[i] += 1;
				}
				else
					grad[i] += pos[i] / l;
			}

			return grad;*/
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			IGeometry<T>::NormalDifferential& dn) const
		{
			//vec3 posRel = pos / extent;


		}


		virtual vec2 getUV(const vec3& pos) const override
		{
			//vec3 posLocal = pos;
			vec3 posLocal = projectPosToObject(pos);

			vec3 dist = glm::abs(posLocal) - vec3(extent);
			vec3 distAbs = glm::abs(dist);

			vec2 uv;

			int axisU, axisV;

			if (distAbs.x <= distAbs.y && distAbs.x <= distAbs.z) // x side
			{
				axisU = 2;
				axisV = 1;
			}
			else if (distAbs.y <= distAbs.z) // y side
			{
				axisU = 0;
				axisV = 2;
			}
			else // z side
			{
				axisU = 0;
				axisV = 1;
			}

			uv.x = (posLocal[axisU] / extent[axisU] + (T)1.0) * (T)0.5;
			uv.y = (posLocal[axisV] / extent[axisV] + (T)1.0) * (T)0.5;

			return glm::clamp(uv, vec2(0), vec2(1));
		}

		virtual SignedDistanceGeometry<T>::SdfType getSdfType() const
		{
			return ST_Box;
		}

		virtual void drawSdfDebugGui() override
		{
			Gui::Property("Extent", extent);
		}
	};
}