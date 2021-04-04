#pragma once

#include "VoxelInterpolator.h"

#include <array>

namespace VoxelGI
{
	template <typename T>
	class VoxelInterpolatorTrilinear : public VoxelInterpolator<T>
	{
	private:
		void getInterpolationVectorTrilinear(const vec3& t, std::array<T, 8>& tvec) const
		{
			getInterpolationVectorTrilinear(t, vec3(1) - t, tvec);
		}

		void getInterpolationVectorTrilinear(const vec3& t, const vec3& nt, std::array<T, 8>& tvec) const
		{
			tvec =
			{
				nt.x * nt.y * nt.z,
				t.x * nt.y * nt.z,
				nt.x * t.y * nt.z,
				t.x * t.y * nt.z,
				nt.x * nt.y * t.z,
				t.x * nt.y * t.z,
				nt.x * t.y * t.z,
				t.x * t.y * t.z
			};
		}

		void calcGradientDiff(const std::array<T, 8>& c, const vec3& t, const vec3& dpdx, vec3& dndx) const
		{
			std::array<T, 8> tvec[3][2];

			vec3 factor = (vec3(getResolution()) / (T)scale) * (vec3(getResolution()) / (T)scale);

			getInterpolationVectorTrilinear(vec3(1, dpdx.y, t.z), vec3(-1, -dpdx.y, 1 - t.z), tvec[0][0]);
			getInterpolationVectorTrilinear(vec3(1, t.y, dpdx.z), vec3(-1, 1 - t.y, -dpdx.z), tvec[0][1]);

			getInterpolationVectorTrilinear(vec3(dpdx.x, 1, t.z), vec3(-dpdx.x, -1, 1 - t.z), tvec[1][0]);
			getInterpolationVectorTrilinear(vec3(t.x, 1, dpdx.z), vec3(1 - t.x, -1, -dpdx.z), tvec[1][1]);

			getInterpolationVectorTrilinear(vec3(dpdx.x, t.y, 1), vec3(-dpdx.x, 1 - t.y, -1), tvec[2][0]);
			getInterpolationVectorTrilinear(vec3(t.x, dpdx.y, 1), vec3(1 - t.x, -dpdx.y, -1), tvec[2][1]);

			dndx = vec3(0);

			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 8; j++)
					dndx[i] += (tvec[i][0][j] + tvec[i][1][j]) * c[j];

			//dndx *= (vec3(getResolution()) / (T)scale);
			dndx *= factor;
		}

	public:
		virtual T filter(const vec3& pos) const
		{
			T interpolated = (T)0;

			vec3 t;

			std::array<T, 8> c;
			std::array<T, 8> tvec;

			t = getNeighborCells<2>(coordsRelToAbs(pos), c);

			getInterpolationVectorTrilinear(t, tvec);

			for (int i = 0; i < 8; i++)
				interpolated += tvec[i] * c[i];

			return interpolated;
		}

		virtual vec3 gradient(const vec3& pos) const
		{
			vec3 posBuffer = coordsRelToAbs(pos);

			vec3 grad, t;
			std::array<T, 8> c, tvec[3];

			t = getNeighborCells<2>(posBuffer, c);

			getInterpolationVectorTrilinear(vec3(1, t.y, t.z), vec3(-1, 1 - t.y, 1 - t.z), tvec[0]);
			getInterpolationVectorTrilinear(vec3(t.x, 1, t.z), vec3(1 - t.x, -1, 1 - t.z), tvec[1]);
			getInterpolationVectorTrilinear(vec3(t.x, t.y, 1), vec3(1 - t.x, 1 - t.y, -1), tvec[2]);

			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 8; j++)
					grad[i] += tvec[i][j] * c[j];

			grad *= vec3(getResolution()) / (T)scale;

			return glm::normalize(grad);
		}

		virtual void gradientDifferential(
			const vec3& pos,
			const RayDifferential<Float>& rayDiff,
			typename IGeometry<T>::NormalDifferential& dn) const
		{
			vec3 posBuffer, t;

			std::array<T, 8> c;

			dn.n = gradient(pos);

			posBuffer = coordsRelToAbs(pos);

			t = getNeighborCells<2>(posBuffer, c);

			calcGradientDiff(c, t, rayDiff.getDpdx(), dn.dndx);
			calcGradientDiff(c, t, rayDiff.getDpdy(), dn.dndy);

			dn.computeDN(true);
		}


		// Experimental

		virtual vec3 gradientInterpolated(const vec3& pos) const
		{
			/*glm::vec3 normal;

			std::array<vec3, 8> normals;
			std::array<T, 8> tvec;
			std::array<glm::ivec3, 8> indices;

			vec3 posBuffer = coordsRelToAbs(pos);

			glm::ivec3 lowerCell = glm::floor(posBuffer);

			getCellIndices<2>(lowerCell, glm::ivec3(0), getResolution(), indices);

			for (int i = 0; i < 8; i++)
				normals[i] = gradient(coordsAbsToRel(vec3(indices[i])));

			vec3 t = posBuffer - vec3(lowerCell);

			getInterpolationVectorTrilinear(t, tvec);

			for (int i = 0; i < 8; i++)
				normal += tvec[i] * normals[i];

			return glm::normalize(normal);*/

			return vec3();
		}

		virtual void gradientInterpolatedDifferential(
			const vec3& pos,
			const RayDifferential<Float>& rayDiff,
			typename IGeometry<T>::NormalDifferential& dn) const
		{
			/*std::array<vec3, 8> normals;
			std::array<T, 8> tvec[3];
			std::array<glm::ivec3, 8> indices;

			vec3 posBuffer = coordsRelToAbs(pos);

			glm::ivec3 lowerCell = glm::floor(posBuffer);

			vec3 t = posBuffer - vec3(lowerCell);
			//vec3 dtdx = projectToObjectAbsDiff(projectPosDiffToObject(rayDiff.getDpdx()));
			vec3 dtdx = rayDiff.getDpdx() / (Float)scale * (T)0.5 * vec3(getResolution());

			getCellIndices<2>(lowerCell, glm::ivec3(0), getResolution(), indices);

			for (int i = 0; i < 8; i++)
				normals[i] = gradient(coordsAbsToRel(vec3(indices[i])));

			getInterpolationVectorTrilinear(vec3(dtdx.x, t.y, t.z), vec3(-dtdx.x, 1 - t.y, 1 - t.z), tvec[0]);
			getInterpolationVectorTrilinear(vec3(t.x, dtdx.y, t.z), vec3(1 - t.x, -dtdx.y, 1 - t.z), tvec[1]);
			getInterpolationVectorTrilinear(vec3(t.x, t.y, dtdx.z), vec3(1 - t.x, 1 - t.y, -dtdx.z), tvec[2]);

			dn.n = gradientInterpolated(pos);

			for (int i = 0; i < 8; i++)
				dn.dndx += (tvec[0][i] + tvec[1][i] + tvec[2][i]) * normals[i];

			dn.computeDN(true);*/
		}
	};
}