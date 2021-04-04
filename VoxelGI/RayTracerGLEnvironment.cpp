#include "RayTracerGLEnvironment.h"
#include "SignedDistanceSphere.h"
#include "VoxelGeometry.h"

namespace VoxelGI
{
	RayTracerGLEnvironment::RayTracerGLEnvironment(VoxelGI & engine)
		: voxelGI(engine), cameraControl(voxelGI, camera)
	{
	}

	RayTracerGLEnvironment::~RayTracerGLEnvironment()
	{
	}

	void RayTracerGLEnvironment::init()
	{
		shader.createFromFile("RayTracerGL.vert", "RayTracerGL.frag");

		quadMesh.create(MP_Quad);

		camera.setTranslation(glm::vec3(0.0f, 0.0f, 3.0f));

		VoxelGeometry<float> voxelGeo(glm::ivec3(64, 64, 64));
		voxelGeo.create(SignedDistanceSphere<float>(0.5f));

		//texture.createFromBuffer(GL_TEXTURE_3D, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR, glm::ivec3(voxelGeo.getResolution()), (void*)voxelGeo.buffer().data());
	}

	void RayTracerGLEnvironment::update()
	{
		guiUpdate();

		cameraControl.update();
	}

	void RayTracerGLEnvironment::render()
	{
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.bind();
		
		shader.setUniform("scale", glm::vec2(1.0f, 1.0f / voxelGI.getAspect()));
		shader.setUniform("viewMat", camera.getViewMatrix());
		shader.setUniform("voxelData", texture, 0);

		quadMesh.draw();
	}

	Camera & RayTracerGLEnvironment::getCamera()
	{
		return Camera();
	}

	void RayTracerGLEnvironment::guiUpdate()
	{
	}
}