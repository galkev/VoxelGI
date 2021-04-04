#include "VoxelNode.h"

#include <algorithm>
#include <glm/gtc/random.hpp>

namespace VoxelGI
{

	VoxelNode::VoxelNode()
	{

	}


	VoxelNode::~VoxelNode()
	{
	}

	void VoxelNode::create(const SceneNode& sceneNode)
	{

	}

	const Texture & VoxelNode::data() const
	{
		return voxelData;
	}

	void VoxelNode::createPrimitive(PrimitiveType primitiveType, const glm::ivec3 & res)
	{
		/*TextureBuffer texBuffer;

		texBuffer.resize(res.x, res.y, res.z);

		switch (primitiveType)
		{
		case PT_Sphere:
		{
			float radius = std::min(res.x, std::min(res.y, res.z)) * 0.5f;

			for (int x = 0; x < res.x; x++)
				for (int y = 0; y < res.y; y++)
					for (int z = 0; z < res.z; z++)
					{
						if (glm::distance(glm::vec3(x, y, z), glm::vec3(res) * 0.5f) <= radius)
						{
							glm::vec4 color = glm::linearRand(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
							texBuffer.setPixel(glm::ivec3(x, y, z), color);
						}
						else
							texBuffer.setPixel(glm::ivec3(x, y, z), glm::vec4(0.0f));
					}
			break;
		}
		}

		voxelData.createFromBuffer(GL_TEXTURE_3D, res, texBuffer);*/
	}

}