#pragma once

#include "VoxelInterpolator.h"

#include <array>

#include "had\had.h"

#include "glm/gtx/string_cast.hpp"

namespace VoxelGI
{
	template <
		typename T, 
		int N, 
		T (*kernelFuncFloatType)(const std::array<T, N>&, T),
		had::AReal (*kernelFuncAdType)(const std::array<had::AReal, N>&, had::AReal)>
	class VoxelInterpolatorKernel : public VoxelInterpolator<T>
	{
	public:
		static constexpr int kernelWidth = N;
		static constexpr int kernelVolume = kernelWidth * kernelWidth * kernelWidth;

	private:
		T kernel(const std::array<T, kernelWidth>& values, T t) const
		{
			return kernelFuncFloatType(values, t);
		}

		had::AReal kernel(const std::array<had::AReal, kernelWidth>& values, had::AReal t) const
		{
			return kernelFuncAdType(values, t);
		}

		template <typename U>
		U applyKernel(const std::array<T, kernelVolume>& values, const glm::tvec3<U>& t) const
		{
			U interpolated = (U)0;
			std::array<std::array<U, kernelWidth>, 3> interpolValues;

			for (int z = 0; z < kernelWidth; z++)
			{
				for (int y = 0; y < kernelWidth; y++)
				{
					for (int x = 0; x < kernelWidth; x++)
						interpolValues[0][x] = bufferAt<kernelWidth>(values, x, y, z);

					interpolValues[1][y] = kernel(interpolValues[0], t.x);
				}

				interpolValues[2][z] = kernel(interpolValues[1], t.y);
			}

			interpolated = kernel(interpolValues[2], t.z);

			return interpolated;
		}

		template <typename U>
		void applyKernelGradient(
			const std::array<T, kernelVolume>& values,
			const glm::tvec3<U>& t,
			vec3& gradientVec) const
		{
			had::ADGraph adGraph;

			glm::tvec3<had::AReal> dt = t;
			had::AReal val = applyKernel(values, dt);

			had::SetAdjoint(val, 1.0);
			had::PropagateAdjoint();

			gradientVec.x = GetAdjoint(dt.x);
			gradientVec.y = GetAdjoint(dt.y);
			gradientVec.z = GetAdjoint(dt.z);

			adGraph.Clear();
		}

		template <typename U>
		void applyKernelHessian(
			const std::array<T, kernelVolume>& values,
			const glm::tvec3<U>& t,
			vec3& gradientVec,
			glm::tmat3x3<T>& hessianMat) const
		{
			had::ADGraph adGraph;

			glm::tvec3<had::AReal> dt = t;
			had::AReal val = applyKernel(values, dt);

			had::SetAdjoint(val, 1.0);
			had::PropagateAdjoint();

			gradientVec.x = GetAdjoint(dt.x);
			gradientVec.y = GetAdjoint(dt.y);
			gradientVec.z = GetAdjoint(dt.z);

			T dxx = had::GetAdjoint(dt.x, dt.x);
			T dxy = had::GetAdjoint(dt.x, dt.y);
			T dxz = had::GetAdjoint(dt.x, dt.z);
			T dyy = had::GetAdjoint(dt.y, dt.y);
			T dyz = had::GetAdjoint(dt.y, dt.z);
			T dzz = had::GetAdjoint(dt.z, dt.z);

			hessianMat = glm::tmat3x3<T>(
				dxx, dxy, dxz,
				dxy, dyy, dyz,
				dxz, dyz, dzz
				);

			adGraph.Clear();
		}

	public:
		virtual T filter(const vec3& pos) const
		{
			std::array<T, kernelVolume> values;

			vec3 t = getNeighborCells<kernelWidth>(coordsRelToAbs(pos), values);

			return applyKernel(values, t);
		}

		virtual vec3 gradient(const vec3& pos) const
		{
			std::array<T, kernelVolume> values;

			vec3 t = getNeighborCells<kernelWidth>(coordsRelToAbs(pos), values);

			vec3 gradientVec;

			applyKernelGradient(values, t, gradientVec);

			return gradientVec;
		}

		virtual void hessian(
			const vec3& pos,
			vec3& gradientVec,
			glm::tmat3x3<T>& hessianMat) const
		{
			std::array<T, kernelVolume> values;

			vec3 t = getNeighborCells<kernelWidth>(coordsRelToAbs(pos), values);

			vec3 dtdp = vec3(getResolution()) / (T)scale;

			applyKernelHessian(values, t, gradientVec, hessianMat);
		}
	};
}