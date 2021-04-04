#include "Mesh.h"

namespace VoxelGI
{

	Mesh::Mesh()
	{
	}


	Mesh::~Mesh()
	{
		release();
	}

	void Mesh::create(MeshPrimitive meshPrimitive)
	{
		const GLfloat r = 1.0f;

		switch (meshPrimitive)
		{
		case MP_Tri:
		{
			GLfloat vertexBuffer[] =
			{
				0.0f, r,  0.0f,
				-r, -r,  0.0f,
				r,  -r,  0.0f,
			};

			create(vertexBuffer, 4, 3, GL_TRIANGLES);
			break;
		}
		case MP_Quad:
		{
			GLfloat vertexBuffer[] =
			{
				// front
				-r, r,  0.0f,
				-r, -r,  0.0f,
				r,  r,  0.0f,
				r,  -r,  0.0f,
			};

			create(vertexBuffer, 4, 3, GL_TRIANGLE_STRIP);
			break;
		}
		case MP_Cube:
		{
			GLfloat vertexBuffer[] =
			{
				// front
				-r, -r,  r,
				r, -r,  r,
				r,  r,  r,
				-r,  r,  r,
				// back
				-r, -r, -r,
				r, -r, -r,
				r,  r, -r,
				-r,  r, -r,
			};

			GLuint indexBuffer[] =
			{
				// front
				0, 1, 2,
				2, 3, 0,
				// top
				1, 5, 6,
				6, 2, 1,
				// back
				7, 6, 5,
				5, 4, 7,
				// bottom
				4, 0, 3,
				3, 7, 4,
				// left
				4, 5, 1,
				1, 0, 4,
				// right
				3, 2, 6,
				6, 7, 3,
			};

			//create(vertexBuffer, indexBuffer, 8, 3, 36);

			break;
		}
		}
	}

	void Mesh::create(GLfloat * vertices, int vertexCount, int vertexElementSize, GLenum drawMode)
	{
		vertexArray.create(vertices, vertexCount, vertexElementSize, drawMode);
	}

	void Mesh::create(GLfloat * vertices, GLfloat * normals, GLuint * indices, int vertexCount, int vertexElementSize, int indexCount)
	{
		vertexArray.create(vertices, normals, indices, vertexCount, vertexElementSize, indexCount, VertexArray::ST_SplitSperate);
	}

	void Mesh::draw()
	{
		vertexArray.bind();

		vertexArray.draw();

		vertexArray.unbind();
	}

	void Mesh::release()
	{
		vertexArray.release();
	}

}