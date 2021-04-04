#pragma once

#include <glm\glm.hpp>

namespace VoxelGI
{
	enum VoxelAttribute
	{
		VA_SignedDistance,
		VA_UV,
		VA_U,
		VA_V,
		VA_Normal,
		VA_All
	};

	template <typename T>
	class VoxelCell
	{
	public:	
		T signedDistance = 0.0f;
		glm::tvec2<T> uv;
		//glm::tvec3<T> normal;

		VoxelCell() {}
		VoxelCell(T sdf) : signedDistance(sdf) {}
		//VoxelCell(T sdf, const glm::tvec3<T>& n) : signedDistance(sdf), normal(n) {}

		template <VoxelAttribute attr, typename U>
		U& get();

		template <>
		float& get<VA_SignedDistance, float>()
		{
			return signedDistance;
		}

		template <>
		double& get<VA_SignedDistance, double>()
		{
			return signedDistance;
		}

		template <>
		float& get<VA_UV, float>()
		{
			return uv;
		}

		template <>
		double& get<VA_UV, double>()
		{
			return uv;
		}

		template <>
		float& get<VA_U, float>()
		{
			return uv.x;
		}

		template <>
		double& get<VA_U, double>()
		{
			return uv,x;
		}

		template <>
		float& get<VA_V, float>()
		{
			return uv.y;
		}

		template <>
		double& get<VA_V, double>()
		{
			return uv.y;
		}

		/*template <>
		glm::vec3 get<VA_Normal, glm::vec3>()
		{
			return normal;
		}

		template <>
		glm::tvec3<double> get<VA_Normal, glm::tvec3<double>>()
		{
			return normal;
		}*/

		template <>
		VoxelCell& get<VA_All, VoxelCell>()
		{
			return *this;
		}

		void setNormal(const glm::tvec3<T>& n)
		{
			//normal = n;
		}

		VoxelCell& operator+=(const VoxelCell& voxelCell)
		{
			*this = *this + voxelCell;
			return *this;
		}

		//friend VoxelCell operator+(const VoxelCell&, const VoxelCell&);
		//friend VoxelCell operator-(const VoxelCell&, const VoxelCell&);

		template <typename T>
		friend VoxelCell operator*(T, const VoxelCell&);
	};

	template <typename T>
	inline VoxelCell<T> operator+(const VoxelCell<T>& v1, const VoxelCell<T>& v2)
	{
		return VoxelCell<T>(v1.signedDistance + v2.signedDistance);
		//return VoxelCell<T>(v1.signedDistance + v2.signedDistance, v1.normal + v2.normal);
	}

	template <typename T>
	inline VoxelCell<T> operator-(const VoxelCell<T>& v1, const VoxelCell<T>& v2)
	{
		return VoxelCell<T>(v1.signedDistance - v2.signedDistance);
		//return VoxelCell<T>(v1.signedDistance - v2.signedDistance, v1.normal - v2.normal);
	}

	/*template <typename T>
	inline VoxelCell operator*(const VoxelCell& v1, T s)
	{
		return VoxelCell(v1.signedDistance * (float)s, v1.normal * (float)s);
	}*/

	template <typename T>
	inline VoxelCell<T> operator*(T s, const VoxelCell<T>& v1)
	{
		return VoxelCell<T>(v1.signedDistance * (T)s);
		//return VoxelCell<T>(v1.signedDistance * (T)s, v1.normal * (T)s);
	}
}

