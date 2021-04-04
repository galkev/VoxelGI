#pragma once

#include "Texture2DCpu.h"
#include <glm/glm.hpp>

namespace VoxelGI
{
	class VoxelGI;

	template <typename T>
	class RayDifferential;

	class IEnvironment;

	struct Material
	{
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
		bool isMirror = false;
		unsigned char texId;
	};

	struct Light
	{
		enum LightType
		{
			LT_Point,
			LT_Directional
		};

		glm::vec3 pos;
		glm::vec3 dir;
		glm::vec3 color;
		glm::vec3 ambient;
		float power;
		LightType lightType;
	};

	template <typename T>
	class IGeometry
	{
	public:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;

		struct NormalDifferential;

		enum GeometryType
		{
			GT_Triangle,
			GT_Sphere,
			GT_Sdf,
		};

		virtual ~IGeometry() {}

		virtual vec3 surfaceNormal(const vec3& pos) const = 0;
		virtual vec3 shadingNormal(const vec3& pos) const = 0;

		virtual glm::vec4 surfaceColor(const RayDifferential<Float>& rayDiff) const = 0;

		virtual void getMaterial(const vec3& position, Material& materialOut) const = 0;
		virtual vec2 getUV(const vec3& pos) const = 0;

		virtual void copy(IGeometry<T>& geoOut) const = 0;
		virtual void transform(const glm::mat4& transformMat) = 0;

		// Differentials
		virtual void getDt(const RayDifferential<Float>& rayDiff, Float& dtdx, Float& dtdy) const = 0;

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			NormalDifferential& dn) const = 0;

		virtual void getDuv(
			const RayDifferential<Float>& rayDiff,
			vec2& duvdx,
			vec2& duvdy) const = 0;

		// ray intersect
		virtual bool transfer(const Ray<T>& ray) const = 0;

		virtual GeometryType geoType() const = 0;

		virtual void drawDebugGui() = 0;

		virtual void setTexture(const std::shared_ptr<Texture2DCpu<T>>& tex) = 0;

		virtual void setEnvironment(IEnvironment* env) = 0;

		virtual bool getTextureValue(const RayDifferential<Float>& rayDiff, int texId, glm::vec4& texValOut) const = 0;
	};

	template <typename T>
	class Geometry : public IGeometry<T>
	{
	protected:
		IEnvironment * environment = nullptr;
		Material defaultMaterial;

	public:
		virtual void setEnvironment(IEnvironment* env)
		{
			environment = env;
		}

		// Differentials
		virtual void getDt(const RayDifferential<Float>& rayDiff, Float& dtdx, Float& dtdy) const
		{
			vec3 normal = surfaceNormal(rayDiff.getPosition());

			dtdx = -glm::dot(rayDiff.getDpdx() + rayDiff.t * rayDiff.getDddx(), normal) / glm::dot(rayDiff.direction, vec3(normal));
			dtdy = -glm::dot(rayDiff.getDpdy() + rayDiff.t * rayDiff.getDddy(), normal) / glm::dot(rayDiff.direction, vec3(normal));
		}

		virtual void drawDebugGui() {}

		// TEMP
		virtual void setTexture(const std::shared_ptr<Texture2DCpu<T>>& tex) {};
		virtual void getMaterial(const vec3& position, Material& materialOut) const { materialOut = defaultMaterial; }
		virtual void getDuv(
			const RayDifferential<Float>& rayDiff,
			vec2& duvdx,
			vec2& duvdy) const {}
		virtual bool getTextureValue(const RayDifferential<Float>& rayDiff, int texId, glm::vec4& texValOut) const { return false; }
	};

	template <typename T>
	struct IGeometry<T>::NormalDifferential
	{
		vec3 n;
		vec3 N;
		vec3 dndx;
		vec3 dndy;
		vec3 dNdx;
		vec3 dNdy;

		NormalDifferential() {}
		NormalDifferential(const vec3& sn, const vec3& sdndx, const vec3& sdndy, bool doNormalize = true)
			: n(sn), dndx(sdndx), dndy(sdndy)
		{
			computeDN(doNormalize);
		}

		void computeDN(bool normalize)
		{
			if (normalize)
			{
				N = glm::normalize(n);
				dNdx = (glm::dot(n, n) * dndx - glm::dot(n, dndx) * n) / std::pow(glm::dot(n, n), (T)1.5);
				dNdy = (glm::dot(n, n) * dndy - glm::dot(n, dndy) * n) / std::pow(glm::dot(n, n), (T)1.5);
			}
			else
			{
				N = n;
				dNdx = dndx;
				dNdy = dndy;
			}
		}
	};

	template <typename T>
	struct Bounds
	{
		using vec3 = glm::tvec3<T>;

		vec3 lower;
		vec3 upper;

		T volumeSq()
		{
			return glm::distance2(lower, upper);
		}

		T volume()
		{
			return glm::distance(lower, uppder);
		}

		bool contains(const vec3& pos)
		{
			return lower.x <= pos.x && pos.x <= upper.x
				&& lower.y <= pos.y && pos.y <= upper.y
				&& lower.z <= pos.z && pos.z <= upper.z;
		}
	};
}