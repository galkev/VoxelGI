#include "CameraControl.h"
#include <glm/gtx/rotate_vector.hpp>

namespace VoxelGI
{
	CameraControl::CameraControl(VoxelGI& engine, Camera& cam)
		: voxelGI(engine), camera(cam)
	{
	}

	void CameraControl::update()
	{
		glm::vec3 camTrans;
		glm::vec2 camRot;

		if (!freezeCamera)
		{
			// Move forward
			if (voxelGI.getInput().getKey(GLFW_KEY_W)) {
				camTrans.z = voxelGI.getTimer().deltaTime() * moveSpeed;
			}
			// Move backward
			if (voxelGI.getInput().getKey(GLFW_KEY_S)) {
				camTrans.z = -voxelGI.getTimer().deltaTime() * moveSpeed;
			}
			// Strafe right
			if (voxelGI.getInput().getKey(GLFW_KEY_D)) {
				camTrans.x = voxelGI.getTimer().deltaTime() * moveSpeed;
			}
			// Strafe left
			if (voxelGI.getInput().getKey(GLFW_KEY_A)) {
				camTrans.x = -voxelGI.getTimer().deltaTime() * moveSpeed;
			}
			// Move up
			if (voxelGI.getInput().getKey(GLFW_KEY_Q)) {
				camTrans.y = voxelGI.getTimer().deltaTime() * moveSpeed;
			}
			// Move down
			if (voxelGI.getInput().getKey(GLFW_KEY_E)) {
				camTrans.y = -voxelGI.getTimer().deltaTime() * moveSpeed;
			}

			// Move forward
			if (voxelGI.getInput().getKey(GLFW_KEY_UP)) {
				camRot.x -= voxelGI.getTimer().deltaTime() * rotSpeed;
			}
			// Move backward
			if (voxelGI.getInput().getKey(GLFW_KEY_DOWN)) {
				camRot.x += voxelGI.getTimer().deltaTime() * rotSpeed;
			}
			// Strafe right
			if (voxelGI.getInput().getKey(GLFW_KEY_RIGHT)) {
				camRot.y -= voxelGI.getTimer().deltaTime() * rotSpeed;
			}
			// Strafe left
			if (voxelGI.getInput().getKey(GLFW_KEY_LEFT)) {
				camRot.y += voxelGI.getTimer().deltaTime() * rotSpeed;
			}
		}

		if (voxelGI.getInput().getKeyUp(GLFW_KEY_SPACE))
			freezeCamera = !freezeCamera;


		glm::vec3 camDir = glm::rotateY(camera.forward(), camRot.y);
		camDir = glm::rotate(camDir, -camRot.x, glm::cross(camDir, glm::vec3(0.0f, 1.0f, 0.0f)));

		camera.moveForward(camTrans);
		camera.setDirection(camDir);
	}

}