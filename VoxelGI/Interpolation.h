#pragma once

#include <array>

namespace VoxelGI
{
	enum InterpolationType
	{
		IT_Linear,
		IT_Cubic
	};

	template <typename T>
	class Interpolation
	{
	private:
		static constexpr int kernelW[2] =
		{
			2,
			4
		};

	public:
		template <InterpolationType interpolationType>
		static constexpr int kernelWidth()
		{
			return kernelW[interpolationType];
		}
		
		template <InterpolationType interpolationType, int N>
		static T interpolate(const std::array<T, N>& values, T t)
		{
			static_assert(false, "template error");
		}

		template <>
		static T interpolate<IT_Linear, 2>(const std::array<T, 2>& values, T t)
		{
			return linear(values, t);
		}

		template <>
		static T interpolate<IT_Cubic, 4>(const std::array<T, 4>& values, T t)
		{
			return cubic(values, t);
		}

		static T linear(const std::array<T, 2>& values, T t)
		{
			return (1 - t) * values[0] + t * values[1];
		}

		static T cubic(const std::array<T, 4>& values, T t)
		{
			T A = values[0];
			T B = values[1];
			T C = values[2];
			T D = values[3];

			T a = -A / 2.0 + (3.0*B) / 2.0 - (3.0*C) / 2.0 + D / 2.0;
			T b = A - (5.0 * B) / 2.0 + 2.0*C - D / 2.0;
			T c = -A / 2.0 + C / 2.0;
			T d = B;

			return a * t*t*t + b * t*t + c * t + d;
		}
	};
}