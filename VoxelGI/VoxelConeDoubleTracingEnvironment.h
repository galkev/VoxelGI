#pragma once

#include "Environment.h"
#include "Shader.h"
#include "Camera.h"
#include "SceneNode.h"
#include "VoxelNode.h"
#include "VoxelVisualisation.h"

namespace VoxelGI
{

	class VoxelGI;

	class VoxelConeDoubleTracingEnvironment : public IEnvironment
	{
	private:
		VoxelGI& voxelGI;

		Shader giShader;

		Camera camera;
		glm::vec3 camTarget;

		Texture dataTexture;

		SceneNode cubeNode;
		Mesh cubeMesh;

		SceneNode sphereNode;
		Mesh sphereMesh;

		VoxelNode voxelNode;

		VoxelVisualisation voxelVisualisation;
		bool voxelVisualisationEnabled;

	public:
		VoxelConeDoubleTracingEnvironment(VoxelGI& engine);
		~VoxelConeDoubleTracingEnvironment();

		// Inherited via Environment
		virtual void init() override;

		virtual void update() override;

		virtual void render() override;

		virtual Camera& getCamera();

		virtual void onResize(int width, int height) override;
	};

}