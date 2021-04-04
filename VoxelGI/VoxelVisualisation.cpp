#include "VoxelVisualisation.h"

#include "VoxelGI.h"

namespace VoxelGI
{

	VoxelVisualisation::VoxelVisualisation(IEnvironment& env)
		: environment(env)
	{
	}


	VoxelVisualisation::~VoxelVisualisation()
	{
		release();
	}

	void VoxelVisualisation::create()
	{
		GLfloat quadVertices[] =
		{
			-1.0f, -1.0f,
			-1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,
		};

		screenQuad.create(quadVertices, 4, 2, GL_TRIANGLE_STRIP);

		shader.createFromFile("VoxelVisualisation.vert", "VoxelVisualisation.frag");
	}

	void VoxelVisualisation::release()
	{
		screenQuad.release();
	}

	void VoxelVisualisation::draw(const VoxelNode& voxelNode)
	{
		shader.bind();

		//shader.setUniform("camPos", environment.getCamera().translation());
		shader.setUniform("voxelData", voxelNode.data(), 0);

		screenQuad.bind();
		screenQuad.draw();
		screenQuad.unbind();
	}

}