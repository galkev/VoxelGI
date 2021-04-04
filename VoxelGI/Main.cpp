
#include "VoxelGI.h"

int main()
{


	std::cout << "Begin" << std::endl;

	{
		VoxelGI::VoxelGI voxelGI;

		voxelGI.init();

		voxelGI.exit();
	}

	std::cout << "End" << std::endl;

	return 0;
}