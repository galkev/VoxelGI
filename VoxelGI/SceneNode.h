#pragma once

#include "Mesh.h"
#include "NodeTransform.h"
#include "Shader.h"

namespace VoxelGI
{

	class IEnvironment;

	class SceneNode
	{
	private:
		const std::string attribPos = "dsa";

		Mesh mesh;
		NodeTransform nodeTransform;
		Shader shader;

		IEnvironment& environment;

	public:
		SceneNode(IEnvironment& env);
		~SceneNode();

		void create(const Mesh& nodeMesh, const Shader& nodeShader);
		void setShader(const Shader& nodeShader);
		void draw();

		NodeTransform& transform();
	};

}