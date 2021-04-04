#pragma once

#include "Mesh.h"
#include <string>
#include <vector>

namespace VoxelGI
{
	struct MeshData
	{
		std::vector<GLfloat> vertices;
		std::vector<GLuint> indices;
		std::vector<GLfloat> normals;
	};

	class MeshLoader
	{
	private:

	public:
		static void load(const std::string& filename, Mesh& outMesh);

		static void load(
			const std::string & filename,
			MeshData& meshData,
			std::vector<int>& shapeOffsets);
	};

}