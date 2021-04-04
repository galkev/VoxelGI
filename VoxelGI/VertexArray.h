#pragma once

#include <GL/glew.h>

namespace VoxelGI
{

	class VertexArray
	{
	public:
		enum StorageType;

	private:
		GLuint vao, vboPos, vboNormal, ebo;
		StorageType storageType;

		int numVertices;
		int vertexSize;
		int numIndices;
		GLenum drawMode;

		GLuint createBuffer(GLenum target, GLsizeiptr size, const GLvoid * data);
		void releaseBuffer(GLuint bo);

		void drawElements();
		void drawArrays();

	public:
		VertexArray();
		~VertexArray();

		void create(GLfloat * vertices, int vertexCount, int vertexElementSize, GLenum mode);
		void create(GLfloat * vertices, GLfloat* normals, GLuint * indices, int vertexCount, int vertexElementSize, int indexCount, StorageType storeType);
		void release();

		void bind();
		void unbind();

		void draw();
	};

	enum VertexArray::StorageType
	{
		ST_SplitSperate,
		ST_SplitCombine,
		ST_Interleave
	};

}