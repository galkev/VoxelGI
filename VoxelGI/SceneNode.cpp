#include "SceneNode.h"

#include "VoxelGI.h"

namespace VoxelGI
{

	SceneNode::SceneNode(IEnvironment& env)
		: environment(env)
	{
	}


	SceneNode::~SceneNode()
	{
	}

	void SceneNode::create(const Mesh& nodeMesh, const Shader& nodeShader)
	{
		mesh = nodeMesh;
		shader = nodeShader;
	}

	void SceneNode::setShader(const Shader & nodeShader)
	{
		shader = nodeShader;
	}

	void SceneNode::draw()
	{
		shader.bind();

		glm::mat4 modelView = environment.getCamera().getViewMatrix() * nodeTransform.getModelMatrix();

		shader.setUniform("modelView", modelView);
		shader.setUniform("projection", environment.getCamera().getProjectionMatrix());

		mesh.draw();
	}

	NodeTransform & SceneNode::transform()
	{
		return nodeTransform;
	}

}