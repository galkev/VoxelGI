#include "NodeTransform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace VoxelGI
{

	NodeTransform::NodeTransform()
	{
	}

	NodeTransform::NodeTransform(const glm::vec3 & trans, const glm::vec3 & rot, const glm::vec3 & scal)
		: translation(trans), rotation(rot), scaling(scal)
	{
	}


	NodeTransform::~NodeTransform()
	{
	}

	void NodeTransform::translate(const glm::vec3 & trans)
	{
		translation += trans;
	}

	void NodeTransform::rotate(const glm::vec3 & rot)
	{
		rotation += rot;
	}

	void NodeTransform::scale(const glm::vec3 & scal)
	{
		scaling *= scal;
	}

	void NodeTransform::setTranslation(const glm::vec3 & trans)
	{
		translation = trans;
	}

	void NodeTransform::setRotation(const glm::vec3 & rot)
	{
		rotation = rot;
	}

	void NodeTransform::setScale(const glm::vec3 & scal)
	{
		scaling = scal;
	}

	glm::vec3 NodeTransform::getTranslation() const
	{
		return translation;
	}

	glm::vec3 NodeTransform::getRotation() const
	{
		return rotation;
	}

	glm::vec3 NodeTransform::getScale() const
	{
		return scaling;
	}

	const glm::mat4 & NodeTransform::getModelMatrix() const
	{
		modelMatrix = glm::translate(glm::scale(glm::mat4(), scaling) * glm::eulerAngleYX(rotation.y, rotation.x), translation);

		return modelMatrix;
	}

	void NodeTransform::getTranslationMatrix(glm::mat4 & transMat) const
	{
		transMat = glm::translate(glm::mat4(), translation);
	}

	void NodeTransform::getRotationMatrix(glm::mat4 & rotMat) const
	{
		rotMat = glm::eulerAngleYX(rotation.y, rotation.x);
	}

	void NodeTransform::getScaleMatrix(glm::mat4 & scaleMat) const
	{
		scaleMat = glm::scale(glm::mat4(), scaling);
	}

}