#pragma once

#include <glm\glm.hpp>

namespace VoxelGI
{

	class NodeTransform
	{
	private:
		glm::vec3 translation;
		glm::vec3 rotation;
		glm::vec3 scaling = glm::vec3(1.0);

		mutable glm::mat4 modelMatrix;

	public:
		NodeTransform();
		NodeTransform(const glm::vec3& trans, const glm::vec3& rot = glm::vec3(0.0f), const glm::vec3& scal = glm::vec3(1.0f));

		~NodeTransform();

		void translate(const glm::vec3& trans);
		void rotate(const glm::vec3& rot);
		void scale(const glm::vec3& scal);

		void setTranslation(const glm::vec3& trans);
		void setRotation(const glm::vec3& rot);
		void setScale(const glm::vec3& scal);

		glm::vec3 getTranslation() const;
		glm::vec3 getRotation() const;
		glm::vec3 getScale() const;

		const glm::mat4& getModelMatrix() const;

		void getTranslationMatrix(glm::mat4& transMat) const;
		void getRotationMatrix(glm::mat4& rotMat) const;
		void getScaleMatrix(glm::mat4& scaleMat) const;
	};

}