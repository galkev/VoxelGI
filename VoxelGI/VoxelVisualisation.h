#pragma once

#include "VertexArray.h"
#include "Shader.h"

#include "VoxelNode.h"

namespace VoxelGI
{

	class VoxelVisualisation
	{
	private:
		IEnvironment& environment;
		VertexArray screenQuad;
		Shader shader;

	public:
		VoxelVisualisation(IEnvironment& env);
		~VoxelVisualisation();

		void create();
		void release();

		void draw(const VoxelNode& voxelNode);
	};

}