#pragma once

#include <glm\glm.hpp>

#include "Vertex.h"
#include "Ray.h"

#include "Plane.h"

#include "Geometry.h"

namespace VoxelGI
{
	template <typename T>
	class Triangle : public Geometry<T>
	{
	private:
		using Float = T;
		using vec2 = glm::tvec2<T, glm::highp>;
		using vec3 = glm::tvec3<T, glm::highp>;
		using vec4 = glm::tvec4<T, glm::highp>;


		Vertex<T>    points[3];
		vec3 triNormal;
		vec4 la, lb, lc;

		vec4 calcL(int pointId)
		{
			vec3& pointA = points[pointId].pos;
			vec3& pointB = points[(pointId + 1) % 3].pos;
			vec3& pointC = points[(pointId + 2) % 3].pos;

			vec3 n = glm::cross(triNormal, pointC - pointB);
			float d = -glm::dot(n, pointB);

			return vec4(n, d) / glm::dot(vec4(n, d), vec4(pointA, 1.0f));
		}

		void initTri()
		{
			triNormal = glm::normalize(glm::cross(points[2].pos - points[0].pos, points[1].pos - points[0].pos));
			preCalcBarycentrics();
		}

		vec3 barycentricInterpolation(
			const vec2& valA,
			const vec2& valB,
			const vec2& valC,
			const vec3& pos) const
		{
			return barycentricInterpolation(vec3(valA, 0.0), vec3(valB, 0.0), vec3(valC, 0.0), pos);
		}

		vec3 barycentricInterpolation(
			const vec3& valA,
			const vec3& valB,
			const vec3& valC,
			const vec3& pos) const
		{
			return vec3(
				glm::dot(vec4(la), vec4(pos, 1.0)) * vec4(valA, 0.0) +
				glm::dot(vec4(lb), vec4(pos, 1.0)) * vec4(valB, 0.0) +
				glm::dot(vec4(lc), vec4(pos, 1.0)) * vec4(valC, 0.0)
			);
		}

		vec3 barycentricInterpolationDiff(
			const vec2& valA,
			const vec2& valB,
			const vec2& valC,
			const vec3& dpdx) const
		{
			return barycentricInterpolationDiff(vec3(valA, 0.0), vec3(valB, 0.0), vec3(valC, 0.0), dpdx);
		}

		vec3 barycentricInterpolationDiff(
			const vec3& valA,
			const vec3& valB,
			const vec3& valC,
			const vec3& dpdx) const
		{
			return glm::dot(vec3(la), dpdx) * valA + glm::dot(vec3(lb), dpdx) * valB + glm::dot(vec3(lc), dpdx) * valC;
		}

	public:
		Triangle() {}
		Triangle(Vertex<T> a, Vertex<T> b, Vertex<T> c) : points{ a, b, c } { initTri(); }

		Triangle(const Triangle& t) 
			: triNormal(t.triNormal), la(t.la), lb(t.lb), lc(t.lc)
		{
			points[0] = t.points[0];
			points[1] = t.points[1];
			points[2] = t.points[2];
		}

		const Vertex<T>& point(int i) const
		{
			return points[i];
		}

		Vertex<T>& point(int i)
		{
			return points[i];
		}

		void preCalcBarycentrics()
		{
			la = calcL(0);
			lb = calcL(1);
			lc = calcL(2);
		}

		void normalizeNormals()
		{
			points[0].normal = glm::normalize(points[0].normal);
			points[1].normal = glm::normalize(points[1].normal);
			points[2].normal = glm::normalize(points[2].normal);
		}

		vec3 surfaceNormal(const vec3& pos) const
		{
			return triNormal;
		}

		vec3 shadingNormal(const vec3& pos) const
		{
			return barycentricInterpolation(point(0).normal, point(1).normal, point(2).normal, pos);
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return barycentricInterpolation(point(0).uv, point(1).uv, point(2).uv, pos);
		}

		virtual glm::vec4 surfaceColor(const RayDifferential<Float>& rayDiff) const
		{
			return vec4(0.0);
		}

		const vec4& lA() const
		{
			return la;
		}

		const vec4& lB() const
		{
			return lb;
		}

		const vec4& lC() const
		{
			return lc;
		}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			Triangle<T>& t = (Triangle<T>&)geoOut;

			t = *this;
		}

		void transform(const glm::mat4& transformMat)
		{
			for (int i = 0; i < 3; i++)
			{
				points[i].pos = vec3(transformMat * vec4(points[i].pos, 1.0f));
				points[i].normal = points[i].normal;
			}

			initTri();
		}

		vec3 calcBarycentricsPlaneL(vec3 point)
		{
			return glm::mat4x3(la[0], lb[0], lc[0], la[1], lb[1], lc[1], la[2], lb[2], lc[2], la[3], lb[3], lc[3])
				* vec4(point, 1.0f);
		}

		vec3 calcBarycentricsCross(vec3 point)
		{
			vec3 bar;

			float triArea = glm::length(glm::cross(points[1].pos - points[0].pos, points[2].pos - points[1].pos));

			bar[0] = glm::length(glm::cross(points[1].pos - point, points[2].pos - point));
			bar[1] = glm::length(glm::cross(points[2].pos - point, points[0].pos - point));
			bar[2] = glm::length(glm::cross(points[0].pos - point, points[1].pos - point));

			bar /= triArea;

			return bar;
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			IGeometry<T>::NormalDifferential& dn) const
		{
			dn.n = vec4(shadingNormal(rayDiff.getPosition()), 0.0);

			dn.dndx = barycentricInterpolationDiff(point(0).normal, point(1).normal, point(2).normal, rayDiff.getDpdx());
			dn.dndy = barycentricInterpolationDiff(point(0).normal, point(1).normal, point(2).normal, rayDiff.getDpdy());

			dn.computeDN(true);
		}

		virtual void getDuv(
			const RayDifferential<Float>& rayDiff,
			vec2& duvdx,
			vec2& duvdy) const
		{
			duvdx = barycentricInterpolationDiff(point(0).uv, point(1).uv, point(2).uv, rayDiff.getDpdx());
			duvdy = barycentricInterpolationDiff(point(0).uv, point(1).uv, point(2).uv, rayDiff.getDpdy());
		}

		bool transfer(const Ray<T>& ray) const
		{
			const Float EPSILON = 0.0000001f;
			vec3 vertex0 = point(0).pos;
			vec3 vertex1 = point(1).pos;
			vec3 vertex2 = point(2).pos;
			vec3 edge1, edge2, h, s, q;
			Float a, f, u, v;
			edge1 = vertex1 - vertex0;
			edge2 = vertex2 - vertex0;

			h = glm::cross(ray.direction, edge2);

			a = glm::dot(edge1, h);
			if (a > -EPSILON && a < EPSILON)
				return false;
			f = 1 / a;
			s = ray.origin - vertex0;

			u = f * (glm::dot(s, h));

			if (u < 0.0 || u > 1.0)
				return false;

			q = glm::cross(s, edge1);

			v = f * glm::dot(ray.direction, q);

			if (v < 0.0 || u + v > 1.0)
				return false;
			// At this stage we can compute t to find out where the intersection point is on the line.
			Float t = f * glm::dot(edge2, q);

			if (t > EPSILON) // ray intersection
			{
				ray.t = t;
				//outIntersectPoint = ray.origin + ray.direction * t;
				return true;
			}
			else // This means that there is a line intersection but not a ray intersection.
				return false;
		}

		IGeometry<T>::GeometryType geoType() const
		{
			return GT_Triangle;
		}

		virtual void drawDebugGui() override
		{
			bool changed = false;

			changed |= Gui::Property("Normal A", points[0].normal);
			changed |= Gui::Property("Normal B", points[1].normal);
			changed |= Gui::Property("Normal C", points[2].normal);

			if (Gui::Button("Normalize"))
			{
				normalizeNormals();
				changed = true;
			}

			if (changed)
				preCalcBarycentrics();
		}
	};

}