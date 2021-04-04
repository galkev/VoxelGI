#pragma once

#include "Environment.h"
#include "Shader.h"
#include "SceneNode.h"

namespace VoxelGI
{

	class TestEnvironment : public IEnvironment
	{
	private:
		VoxelGI& voxelGI;

		Shader shader;

		Texture tex;

		SceneNode quadNode;
		Mesh quadMesh;

		Camera camera;
		glm::vec3 camTarget;

	public:
		TestEnvironment(VoxelGI& engine);
		~TestEnvironment();

		// Inherited via Environment
		virtual void init() override;
		virtual void update() override;
		virtual void render() override;
		virtual Camera & getCamera() override;
	};

}