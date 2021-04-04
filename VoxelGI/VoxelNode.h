#pragma once

#include "Texture.h"
#include "SceneNode.h"

namespace VoxelGI
{

	class VoxelGI;

	class VoxelNode
	{
	public:
		enum PrimitiveType
		{
			PT_Sphere,
			PT_Cube
		};

	private:
		Texture voxelData;

	public:
		VoxelNode();
		~VoxelNode();

		void create(const SceneNode& sceneNode);
		void createPrimitive(PrimitiveType primitiveType, const glm::ivec3& res);

		const Texture& data() const;
	};

}