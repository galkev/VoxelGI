#pragma once

#include <vector>
#include <memory>
#include <glm\glm.hpp>
#include "Triangle.h"
#include "NodeTransform.h"
#include "Vertex.h"

namespace VoxelGI
{
	class IEnvironment;

	template <typename T>
	class ISceneNodeCpu
	{
	public:
		virtual ~ISceneNodeCpu() {}

		virtual NodeTransform& transform() = 0;
		virtual IGeometry<T>* geometry(int i) = 0;
		virtual void transformGeometry(const glm::mat4& viewMatrix) const = 0;
		virtual bool rayIntersect(const Ray<T>& ray, IGeometry<T>*& outGeometry) = 0;
		virtual void drawDebugGui() = 0;
	};

	template <typename T, typename G>
	class SceneNodeCpu : public ISceneNodeCpu<T>
	{
	private:
		IEnvironment& environment;
		NodeTransform nodeTransform;

		std::vector<G>				geo;
		mutable std::vector<G>		geometryTransformed;

	public:
		SceneNodeCpu(IEnvironment& env) : environment(env) {}
		~SceneNodeCpu() {}

		NodeTransform& transform()
		{
			return nodeTransform;	
		}

		IGeometry<T>* geometry(int i)
		{
			return &geo[i];
		}

		void addGeometrySlots(int n)
		{
			geo.resize(geo.size() + n);
		}

		void addGeometry(const G& newGeo)
		{
			geo.push_back(newGeo);
			geo.back().setEnvironment(&environment);
		}

		void transformGeometry(const glm::mat4& viewMatrix) const
		{
			geometryTransformed.resize(geo.size());

			glm::mat4 modelView = viewMatrix * nodeTransform.getModelMatrix();
			glm::mat4 normalMatrix = transpose(inverse(modelView));

			for (int i = 0; i < geo.size(); i++)
			{
				geo[i].copy(geometryTransformed[i]);
				geometryTransformed[i].transform(modelView);
			}
		}
		
		bool rayIntersect(const Ray<T>& ray, IGeometry<T>*& outGeometry)
		{
			for (int i = 0; i < geo.size(); i++)
			{
				if (geometryTransformed[i].transfer(ray))
				{
					outGeometry = &geometryTransformed[i];
					return true;
				}
			}

			return false;
		}

		virtual void drawDebugGui()
		{
			for (int i = 0; i < geo.size(); i++)
			{
				geo[i].drawDebugGui();
			}
		}
	};

}