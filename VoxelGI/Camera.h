#pragma once

#include <glm\glm.hpp>
#include "Ray.h"

namespace VoxelGI
{

	class Camera
	{
	private:
		float fovY;
		float aspect;
		float near;
		float far;

		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;

	public:
		Camera();
		~Camera();

		void translate(const glm::vec3& trans);
		//void rotate(const glm::vec3& rot);

		void lookAt(const glm::vec3& eye, const glm::vec3& center);

		glm::vec3 translation() const;

		glm::vec3 forward() const;
		glm::vec3 right() const;
		glm::vec3 up() const;

		void moveForward(const glm::vec3& amount);

		void setTranslation(const glm::vec3& trans);
		void setDirection(const glm::vec3& dir);
		void setProjection(float projFovY, float projAspect, float projNear, float projFar);

		void setAspect(float projAspect);

		const glm::mat4& getViewMatrix() const;
		const glm::mat4& getProjectionMatrix();

		/*template <typename T>
		Ray<T> createViewRayUnnormalized(T x, T y) const
		{
			using vec3 = glm::tvec3<T>;

			Ray<T> ray;

			ray.origin = translation();
			ray.direction = vec3(forward()) + x * vec3(right()) + y * vec3(up());

			return ray;
		}

		template <typename T>
		Ray<T> createViewRay(T x, T y) const
		{
			Ray<T> ray = createViewRayUnnormalized(x, y);

			ray.normalize();

			return ray;
		}*/
	};
}