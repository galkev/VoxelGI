#pragma once

#include "SignedDistanceGeometry.h"
#include "Mesh.h"

namespace VoxelGI
{
	template <typename T>
	class SignedDistanceMesh : public SignedDistanceGeometry<T>
	{
	private:
		float* meshVertices;
		int meshSize;

	public:
		SignedDistanceMesh() {}
		SignedDistanceMesh(float* vert, int size) : meshVertices(vert), meshSize(size) {}

		virtual void copy(IGeometry<T>& geoOut) const
		{
			SignedDistanceMesh<T>& s = (SignedDistanceMesh<T>&)geoOut;

			s = *this;
		}

		void getTri(int i, std::array<vec3, 3>& tri) const
		{
			tri =
			{
				vec3(meshVertices[i * 9 + 0], meshVertices[i * 9 + 1], meshVertices[i * 9 + 2]),
				vec3(meshVertices[i * 9 + 3], meshVertices[i * 9 + 4], meshVertices[i * 9 + 5]),
				vec3(meshVertices[i * 9 + 6], meshVertices[i * 9 + 7], meshVertices[i * 9 + 8])
			};
		}

		virtual T getSignedDistance(const vec3& pos) const
		{
			/*vec3 closestVertex;
			T closestVertexDistSq = std::numeric_limits<T>::infinity();
			std::vector<int> containingTriIds;

			for (int i = 0; i < meshSize / 3; i++)
			{
				vec3 vertex(meshVertices[i * 3 + 0], meshVertices[i * 3 + 1], meshVertices[i * 3 + 2]);

				T distSq = glm::distance2(pos, vertex);
				
				if (distSq < closestVertexDistSq)
				{
					closestVertexDistSq = distSq;
					closestVertex = vertex;
				}
			}

			for (int i = 0; i < meshSize / 9; i++)
			{
				std::array<vec3, 3> tri;
				getTri(i, tri);

				if (tri[0] == closestVertex || tri[1] == closestVertex || tri[2] == closestVertex)
					containingTriIds.push_back(i);
			}

			T closestTriDistSq = std::numeric_limits<T>::infinity();
			std::vector<int> closestTris;

			for (int i = 0; i < containingTriIds.size(); i++)
			{
				int triId = containingTriIds[i];

				td::array<vec3, 3> tri;
				getTri(i, tri);


			}*/

			const T eps = (T)0.0001;
			std::vector<int> closestTriIndices;
			T closestDistanceSq = std::numeric_limits<T>::infinity();
			vec3 closestPoint;
			

			for (int i = 0; i < meshSize / 9; i++)
			{
				std::array<vec3, 3> tri;
				vec3 triPoint;

				getTri(i, tri);
				T distanceSq = distancePointTriangleSq(pos, tri, triPoint);

				if (glm::distance2(closestPoint, triPoint) < eps * eps)
					closestTriIndices.push_back(i);
				else if (distanceSq < closestDistanceSq)
				{
					closestPoint = triPoint;
					closestDistanceSq = distanceSq;
					closestTriIndices.clear();
					closestTriIndices.push_back(i);
				}
			}

			vec3 pseudoNormal;

			for (int i = 0; i < closestTriIndices.size(); i++)
			{
				std::array<vec3, 3> tri;

				getTri(closestTriIndices[i], tri);

				vec3 triNormal = glm::cross(tri[1] - tri[0], tri[2] - tri[0]);

				pseudoNormal += triNormal;
			}

			/*int sign;

			getTri(closestTriIndex, tri);

			vec3 triNormal = glm::cross(tri[1] - tri[0], tri[2] - tri[0]);*/

			int sign = glm::dot(pseudoNormal, pos - closestPoint) >= 0 ? +1 : -1;

			return sign * std::sqrt(closestDistanceSq);
			//return std::sqrt(distancePointTriangleExactSq(pos, {mesh}))
		}

		virtual vec3 getGradient(const vec3& pos) const
		{
			return vec3();
		}

		virtual void getDn(
			const RayDifferential<Float>& rayDiff,
			NormalDifferential& dn) const
		{
			
		}

		virtual SdfType getSdfType() const
		{
			return ST_Mesh;
		}

		virtual void drawSdfDebugGui() override
		{
			
		}

		virtual vec2 getUV(const vec3& pos) const
		{
			return vec2(-1.0, -1.0);
		}

		/*struct PointTriangleDistanceInfo
		{
			vec3 closestPoint;

			bool isVertex;
			bool isEdge;
			vec3 edge[2];
			vec3 vertex;
		};*/

		T distancePointTriangleSq(const vec3& point, const std::array<vec3, 3> tri, vec3& closestPoint) const
		{
			vec3 diff = point - tri[0];
			vec3 edge0 = tri[1] - tri[0];
			vec3 edge1 = tri[2] - tri[0];
			T a00 = glm::dot(edge0, edge0);
			T a01 = glm::dot(edge0, edge1);
			T a11 = glm::dot(edge1, edge1);
			T b0 = -glm::dot(diff, edge0);
			T b1 = -glm::dot(diff, edge1);
			T const zero = (T)0;
			T const one = (T)1;
			T det = a00 * a11 - a01 * a01;
			T t0 = a01 * b1 - a11 * b0;
			T t1 = a01 * b0 - a00 * b1;

			if (t0 + t1 <= det)
			{
				if (t0 < zero)
				{
					if (t1 < zero)  // region 4
					{
						if (b0 < zero)
						{
							t1 = zero;
							if (-b0 >= a00)  // V1
							{
								t0 = one;
							}
							else  // E01
							{
								t0 = -b0 / a00;
							}
						}
						else
						{
							t0 = zero;
							if (b1 >= zero)  // V0
							{
								t1 = zero;
							}
							else if (-b1 >= a11)  // V2
							{
								t1 = one;
							}
							else  // E20
							{
								t1 = -b1 / a11;
							}
						}
					}
					else  // region 3
					{
						t0 = zero;
						if (b1 >= zero)  // V0
						{
							t1 = zero;
						}
						else if (-b1 >= a11)  // V2
						{
							t1 = one;
						}
						else  // E20
						{
							t1 = -b1 / a11;
						}
					}
				}
				else if (t1 < zero)  // region 5
				{
					t1 = zero;
					if (b0 >= zero)  // V0
					{
						t0 = zero;
					}
					else if (-b0 >= a00)  // V1
					{
						t0 = one;
					}
					else  // E01
					{
						t0 = -b0 / a00;
					}
				}
				else  // region 0, interior
				{
					T invDet = one / det;
					t0 *= invDet;
					t1 *= invDet;
				}
			}
			else
			{
				T tmp0, tmp1, numer, denom;

				if (t0 < zero)  // region 2
				{
					tmp0 = a01 + b0;
					tmp1 = a11 + b1;
					if (tmp1 > tmp0)
					{
						numer = tmp1 - tmp0;
						denom = a00 - ((T)2)*a01 + a11;
						if (numer >= denom)  // V1
						{
							t0 = one;
							t1 = zero;
						}
						else  // E12
						{
							t0 = numer / denom;
							t1 = one - t0;
						}
					}
					else
					{
						t0 = zero;
						if (tmp1 <= zero)  // V2
						{
							t1 = one;
						}
						else if (b1 >= zero)  // V0
						{
							t1 = zero;
						}
						else  // E20
						{
							t1 = -b1 / a11;
						}
					}
				}
				else if (t1 < zero)  // region 6
				{
					tmp0 = a01 + b1;
					tmp1 = a00 + b0;
					if (tmp1 > tmp0)
					{
						numer = tmp1 - tmp0;
						denom = a00 - ((T)2)*a01 + a11;
						if (numer >= denom)  // V2
						{
							t1 = one;
							t0 = zero;
						}
						else  // E12
						{
							t1 = numer / denom;
							t0 = one - t1;
						}
					}
					else
					{
						t1 = zero;
						if (tmp1 <= zero)  // V1
						{
							t0 = one;
						}
						else if (b0 >= zero)  // V0
						{
							t0 = zero;
						}
						else  // E01
						{
							t0 = -b0 / a00;
						}
					}
				}
				else  // region 1
				{
					numer = a11 + b1 - a01 - b0;
					if (numer <= zero)  // V2
					{
						t0 = zero;
						t1 = one;
					}
					else
					{
						denom = a00 - ((T)2)*a01 + a11;
						if (numer >= denom)  // V1
						{
							t0 = one;
							t1 = zero;
						}
						else  // 12
						{
							t0 = numer / denom;
							t1 = one - t0;
						}
					}
				}
			}

			vec3 closest = tri[0] + t0 * edge0 + t1 * edge1;

			diff = point - closest;
			T sqrDistance = glm::dot(diff, diff);

			closestPoint = closest;
			return sqrDistance;
		}
	};
}