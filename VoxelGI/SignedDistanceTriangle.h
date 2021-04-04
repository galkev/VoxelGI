#pragma once

#include "SignedDistanceGeometry.h"
#include "Triangle.h"

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceTriangle : public SignedDistanceGeometry<T>
	{
	private:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;


		Triangle<T> triangle;

		static T dot2(vec3 a)
		{
			return glm::dot(a, a);
		}
		

	public:
		SignedDistanceTriangle() {}
		SignedDistanceTriangle(const Triangle<T>& tri) : triangle(tri) { }

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceTriangle<T>& s = (SignedDistanceTriangle<T>&)geoOut;

			s = *this;
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(-1.0, -1.0);
		}

		float sdTriPrism(vec3 p, vec2 h) const
		{
			vec3 q = glm::abs(p+vec3(0,0,h.y));
			return 
				glm::max(
					q.z - h.y, 
					glm::max(
						q.x * (T)0.866025 + p.y * (T)0.5, 
						- p.y * (T)0.5) 
					- h.x * (T)0.5);
		}

		virtual T getSignedDistance(const vec3& p) const
		{
			vec3 a = triangle.point(0).pos;
			vec3 b = triangle.point(1).pos;
			vec3 c = triangle.point(2).pos;

			vec3 ba = b - a; 
			vec3 pa = p - a;
			vec3 cb = c - b; 
			vec3 pb = p - b;
			vec3 ac = a - c; 
			vec3 pc = p - c;
			vec3 nor = glm::cross(ba, ac);

			return std::sqrt(
				(glm::sign(glm::dot(glm::cross(ba, nor), pa)) +
					glm::sign(glm::dot(glm::cross(cb, nor), pb)) +
					glm::sign(glm::dot(glm::cross(ac, nor), pc))<2.0)
				?
				glm::min(glm::min(
					dot2(ba*glm::clamp(glm::dot(ba, pa) / dot2(ba), (T)0.0, (T)1.0) - pa),
					dot2(cb*glm::clamp(glm::dot(cb, pb) / dot2(cb), (T)0.0, (T)1.0) - pb)),
					dot2(ac*glm::clamp(glm::dot(ac, pc) / dot2(ac), (T)0.0, (T)1.0) - pc))
				:
				glm::dot(nor, pa)*glm::dot(nor, pa) / dot2(nor));
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			return triangle.shadingNormal(vec3(pos.x, pos.y, 0.0));
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			IGeometry<T>::NormalDifferential& dn) const
		{

		}

		virtual vec2 getUV(const vec3 pos) const
		{
			return triangle.getUV(pos);
		}

		virtual SignedDistanceGeometry<T>::SdfType getSdfType() const
		{
			return ST_Triangle;
		}
	};
}