#pragma once

#include "VoxelGI.h"
#include "Camera.h"

namespace VoxelGI
{
	class CameraControl
	{
	private:
		VoxelGI & voxelGI;
		Camera & camera;

		float moveSpeed = 1.0f;
		float rotSpeed = 1.0f;

		bool freezeCamera = false;

	public:
		CameraControl(VoxelGI& engine, Camera& cam);
		
		void update();
	};
}
