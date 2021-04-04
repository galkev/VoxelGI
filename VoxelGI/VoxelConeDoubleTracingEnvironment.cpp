#include "VoxelConeDoubleTracingEnvironment.h"
#include "MeshLoader.h"

#include "VoxelGI.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace VoxelGI
{

	VoxelConeDoubleTracingEnvironment::VoxelConeDoubleTracingEnvironment(VoxelGI& engine)
		: voxelGI(engine),
		cubeNode(*this),
		sphereNode(*this),
		voxelVisualisation(*this)
	{
	}


	VoxelConeDoubleTracingEnvironment::~VoxelConeDoubleTracingEnvironment()
	{
	}

	void VoxelConeDoubleTracingEnvironment::init()
	{
		camera.setTranslation(glm::vec3(0.0f, 0.0f, 5.0f));
		camera.setProjection(45.0f, (float)voxelGI.screenWidth() / voxelGI.screenHeight(), 0.05f, 1000.0f);

		giShader.createFromFile("VoxelGI.vert", "VoxelGI.frag");

		MeshLoader::load("Data/Meshes/cube.obj", cubeMesh);
		cubeNode.create(cubeMesh, giShader);

		MeshLoader::load("Data/Meshes/sphere.obj", sphereMesh);
		sphereNode.create(sphereMesh, giShader);

		sphereNode.transform().setTranslation(glm::vec3(0.0f, 2.0f, 0.0f));

		voxelVisualisation.create();

		voxelNode.createPrimitive(VoxelNode::PT_Sphere, glm::ivec3(32, 32, 32));
	}

	void VoxelConeDoubleTracingEnvironment::update()
	{
		glm::vec3 camTrans;
		glm::vec2 camRot;

		float speed = 1.0f;

		// Move forward
		if (voxelGI.getInput().getKey(GLFW_KEY_UP)) {
			camTrans.y = voxelGI.getTimer().deltaTime() * speed;
		}
		// Move backward
		if (voxelGI.getInput().getKey(GLFW_KEY_DOWN)) {
			camTrans.y = -voxelGI.getTimer().deltaTime() * speed;
		}
		// Strafe right
		if (voxelGI.getInput().getKey(GLFW_KEY_RIGHT)) {
			camTrans.x = voxelGI.getTimer().deltaTime() * speed;
		}
		// Strafe left
		if (voxelGI.getInput().getKey(GLFW_KEY_LEFT)) {
			camTrans.x = -voxelGI.getTimer().deltaTime() * speed;
		}

		camTrans.z = -voxelGI.getInput().scrollY() * voxelGI.getTimer().deltaTime() * speed;


		float rotSpeed = 1.0f;

		// Move forward
		if (voxelGI.getInput().getKey(GLFW_KEY_W)) {
			camRot.x += voxelGI.getTimer().deltaTime() * rotSpeed;
		}
		// Move backward
		if (voxelGI.getInput().getKey(GLFW_KEY_S)) {
			camRot.x -= voxelGI.getTimer().deltaTime() * rotSpeed;
		}
		// Strafe right
		if (voxelGI.getInput().getKey(GLFW_KEY_D)) {
			camRot.y += voxelGI.getTimer().deltaTime() * rotSpeed;
		}
		// Strafe left
		if (voxelGI.getInput().getKey(GLFW_KEY_A)) {
			camRot.y -= voxelGI.getTimer().deltaTime() * rotSpeed;
		}

		const float camDist = 5.0f;

		camTarget += camTrans;

		glm::vec3 camDir = glm::rotateY(camera.forward(), camRot.y);
		camDir = glm::rotate(camDir, -camRot.x, glm::cross(camDir, glm::vec3(0.0f, 1.0f, 0.0f)));

		camera.lookAt(camTarget - camDir * camDist, camTarget);

		if (voxelGI.getInput().getKeyDown(GLFW_KEY_V))
			voxelVisualisationEnabled = !voxelVisualisationEnabled;
	}

	void VoxelConeDoubleTracingEnvironment::render()
	{
		glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		if (voxelVisualisationEnabled)
			voxelVisualisation.draw(voxelNode);
		else
		{
			cubeNode.draw();
			sphereNode.draw();
		}
	}

	Camera & VoxelConeDoubleTracingEnvironment::getCamera()
	{
		return camera;
	}

	void VoxelConeDoubleTracingEnvironment::onResize(int width, int height)
	{
		camera.setAspect((float)width / height);
	}

}