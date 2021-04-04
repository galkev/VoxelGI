
#include "TestEnvironment.h"
#include "VoxelGI.h"

#include <glm/gtx/rotate_vector.hpp>

namespace VoxelGI
{

	TestEnvironment::TestEnvironment(VoxelGI& engine)
		: voxelGI(engine),
		quadNode(*this)
	{
	}


	TestEnvironment::~TestEnvironment()
	{
	}

	void TestEnvironment::init()
	{
		shader.createFromFile("Test.vert", "Test.frag");

		quadMesh.create(MP_Tri);
		quadNode.create(quadMesh, shader);

		camera.setTranslation(glm::vec3(0.0f, 0.0f, 5.0f));
		camera.setProjection(45.0f, (float)voxelGI.screenWidth() / voxelGI.screenHeight(), 0.05f, 1000.0f);
	}

	void TestEnvironment::update()
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
	}

	void TestEnvironment::render()
	{
		glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//shader.setUniform("p_a", )

		quadNode.draw();
	}

	Camera & TestEnvironment::getCamera()
	{
		return camera;
	}

}