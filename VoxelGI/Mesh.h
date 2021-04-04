#pragma once

#include <GL/glew.h>

#include "VertexArray.h"

namespace VoxelGI
{

	enum MeshPrimitive
	{
		MP_Tri,
		MP_Quad,
		MP_Cube,
		MP_Sphere
	};

	class Mesh
	{
	private:
		VertexArray vertexArray;

	public:
		Mesh();
		~Mesh();

		void create(MeshPrimitive meshPrimitive);
		void create(GLfloat * vertices, int vertexCount, int vertexElementSize, GLenum drawMode);
		void create(GLfloat* vertices, GLfloat* normals, GLuint* indices, int vertexCount, int vertexElementSize, int indexCount);
		void draw();

		void release();
	};

}