#pragma once

#include "Environment.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "CameraControl.h"
#include "Texture.h"

namespace VoxelGI
{
	class RayTracerGLEnvironment : public IEnvironment
	{
	private:
		VoxelGI & voxelGI;

		Shader shader;

		Mesh quadMesh;

		Camera camera;
		CameraControl cameraControl;
		Texture texture;

		glm::vec4 backgroundColor = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);

	public:
		RayTracerGLEnvironment(VoxelGI & engine);
		~RayTracerGLEnvironment();

		// Geerbt über IEnvironment
		virtual void init() override;
		virtual void update() override;
		virtual void render() override;
		virtual Camera & getCamera() override;

		void guiUpdate();
	};
}
