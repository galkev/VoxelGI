#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace VoxelGI
{

	Camera::Camera()
	{
		lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}


	Camera::~Camera()
	{
	}

	void Camera::translate(const glm::vec3 & trans)
	{
		viewMatrix[3].x += trans.x;
		viewMatrix[3].y += trans.y;
		viewMatrix[3].z += trans.z;
	}

	/*void Camera::rotate(const glm::vec3 & rot)
	{
		rotation += rot;
	}*/

	void Camera::lookAt(const glm::vec3 & eye, const glm::vec3 & center)
	{
		glm::vec3 fwd = glm::normalize(center - eye);
		glm::vec3 right = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::normalize(glm::cross(right, fwd));

		viewMatrix = glm::mat4(
			glm::vec4(right, 0.0f),
			glm::vec4(up, 0.0f),
			glm::vec4(fwd, 0.0f),
			glm::vec4(eye, 1.0f));
	}

	glm::vec3 Camera::translation() const
	{
		return viewMatrix[3];
	}

	glm::vec3 Camera::forward() const
	{
		return viewMatrix[2];
	}

	glm::vec3 Camera::right() const
	{
		return viewMatrix[0];
	}

	glm::vec3 Camera::up() const
	{
		return viewMatrix[1];
	}

	void Camera::moveForward(const glm::vec3 & amount)
	{
		translate(glm::mat3(viewMatrix) * amount);
	}

	void Camera::setTranslation(const glm::vec3 & trans)
	{
		viewMatrix[3].x = trans.x;
		viewMatrix[3].y = trans.y;
		viewMatrix[3].z = trans.z;
	}

	void Camera::setDirection(const glm::vec3 & dir)
	{
		lookAt(translation(), translation() + dir);
	}

	/*void Camera::setRotation(const glm::vec3 & rot)
	{
		rotation = rot;
	}*/

	void Camera::setProjection(float projFovY, float projAspect, float projNear, float projFar)
	{
		fovY = projFovY;
		aspect = projAspect;
		near = projNear;
		far = projFar;
	}

	void Camera::setAspect(float projAspect)
	{
		aspect = projAspect;
	}

	const glm::mat4& Camera::getViewMatrix() const
	{
		return viewMatrix;
	}

	const glm::mat4& Camera::getProjectionMatrix()
	{
		projMatrix = glm::perspective(fovY, aspect, near, far);

		return projMatrix;
	}
}