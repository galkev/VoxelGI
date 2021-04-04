#include "MeshLoader.h"

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace VoxelGI
{
	void MeshLoader::load(const std::string & filename, Mesh& outMesh)
	{
		std::vector<int> shapeOffsets;
		MeshData meshData;

		load(filename, meshData, shapeOffsets);

		outMesh.create(meshData.vertices.data(), meshData.normals.data(), meshData.indices.data(), 
			meshData.vertices.size() / 3, 3, meshData.indices.size());
	}

	void MeshLoader::load(
		const std::string & filename,
		MeshData& meshData,
		std::vector<int>& shapeOffsets)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());

		if (!ret || !err.empty())
			std::cerr << "Error reading obj file: " << err << std::endl;

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				int fv = shapes[s].mesh.num_face_vertices[f];

				bool hasNormals = !attrib.normals.empty();
				bool hasUVs = !attrib.texcoords.empty();

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					GLuint globalIdx = index_offset + v;
					tinyobj::index_t idx = shapes[s].mesh.indices[globalIdx];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

					if (hasNormals)
					{
						tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
						tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
						tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

						meshData.normals.push_back(nx);
						meshData.normals.push_back(ny);
						meshData.normals.push_back(nz);
					}

					if (hasUVs)
					{
						tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					}
					// Optional: vertex colors
					// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
					// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
					// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];

					meshData.vertices.push_back(vx);
					meshData.vertices.push_back(vy);
					meshData.vertices.push_back(vz);



					meshData.indices.push_back(globalIdx);
				}

				shapeOffsets.push_back(index_offset);
				index_offset += fv;

				// per-face material
				shapes[s].mesh.material_ids[f];
			}
		}
	}

}