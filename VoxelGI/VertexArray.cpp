#include "VertexArray.h"

namespace VoxelGI
{

	VertexArray::VertexArray()
		: vao(0),
		vboPos(0),
		vboNormal(0),
		ebo(0)
	{
	}


	VertexArray::~VertexArray()
	{
		release();
	}

	GLuint VertexArray::createBuffer(GLenum target, GLsizeiptr size, const GLvoid * data)
	{
		GLuint bo;
		glGenBuffers(1, &bo);
		glBindBuffer(target, bo);
		glBufferData(target, size, data, GL_STATIC_DRAW);
		return bo;
	}

	void VertexArray::releaseBuffer(GLuint bo)
	{
		if (bo != 0)
			glDeleteBuffers(1, &bo);
	}

	void VertexArray::create(GLfloat * vertices, int vertexCount, int vertexElementSize, GLenum mode)
	{
		glGenVertexArrays(1, &vao);

		numVertices = vertexCount;
		vertexSize = vertexElementSize;

		drawMode = mode;

		vboPos = createBuffer(GL_ARRAY_BUFFER, numVertices * vertexSize * sizeof(GLfloat), vertices);
	}

	void VertexArray::create(GLfloat * vertices, GLfloat* normals, GLuint * indices, int vertexCount, int vertexElementSize, int indexCount, StorageType storeType)
	{
		glGenVertexArrays(1, &vao);

		numVertices = vertexCount;
		vertexSize = vertexElementSize;
		numIndices = indexCount;
		storageType = storeType;

		vboPos = createBuffer(GL_ARRAY_BUFFER, numVertices * vertexSize * sizeof(GLfloat), vertices);
		vboNormal = createBuffer(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), normals);
		ebo = createBuffer(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices);
	}

	void VertexArray::release()
	{
		if (vao != 0)
			glDeleteVertexArrays(1, &vao);

		releaseBuffer(vboPos);
		releaseBuffer(vboNormal);
		releaseBuffer(ebo);
	}

	void VertexArray::bind()
	{
		glBindVertexArray(vao);

		int posAttribIndex = 0;
		glEnableVertexAttribArray(posAttribIndex);
		glBindBuffer(GL_ARRAY_BUFFER, vboPos);
		glVertexAttribPointer(posAttribIndex, vertexSize, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		if (vboNormal != 0)
		{
			int normalAttribIndex = 1;
			glEnableVertexAttribArray(normalAttribIndex);
			glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
			glVertexAttribPointer(normalAttribIndex, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		}

		if (ebo != 0)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	void VertexArray::unbind()
	{
		int posAttribIndex = 0;
		glDisableVertexAttribArray(posAttribIndex);

		if (vboNormal != 0)
		{
			int normalAttribIndex = 1;
			glDisableVertexAttribArray(normalAttribIndex);
		}
	}

	void VertexArray::draw()
	{
		if (ebo != 0)
			drawElements();
		else
			drawArrays();
	}

	void VertexArray::drawElements()
	{
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
	}

	void VertexArray::drawArrays()
	{
		glDrawArrays(drawMode, 0, numVertices);
	}
}