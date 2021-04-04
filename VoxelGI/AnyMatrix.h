#pragma once

namespace VoxelGI
{
	template <typename T, int numRows, int numColumns>
	class AnyMatrix
	{
	private:
		T a[numRows][numColumns];

	public:
		const int size = numRows * numColumns;

		AnyMatrix() {}
		AnyMatrix(const float* values)
		{
			for (int y = 0; y < numRows; y++)
			{
				for (int x = 0; x < numColumns; x++)
				{
					a[y][x] = values[y * numColumns + x];
				}
			}
		}


	};
}