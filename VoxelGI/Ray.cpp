#include "Ray.h"

#include "RayTracerCpu.h"

namespace VoxelGI
{
	template class Ray<float>;
	template class Ray<double>;

	template class RayDifferential<float>;
	template class RayDifferential<double>;

	//template class RayDifferentialFinite<float>;
	//template class RayDifferentialFinite<double>;
}