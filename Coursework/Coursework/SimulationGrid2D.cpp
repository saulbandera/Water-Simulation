#include "SimulationGrid2D.h"
#include <cmath> // For std::exp and M_PI
#include <iostream>

SimulationGrid2D::SimulationGrid2D(int nx, int ny)
{

	// Setting the grid size
	sizeX = nx;
	sizeY = ny;
	resolution = sizeX * sizeY;

	// Resizing the data structure that will contain the grid data
	grid.resize(ny, std::vector<std::array<float, 4>>(nx));

	// Adding a gaussian pulse to the height values of the grid as initial condition
	float maxHeight = 15.0f; // Height of pulse
	float pulseWidth = std::min(sizeX, sizeY) /8.0f; // Width

	// Used to center the pulse
	int centerX = sizeX / 2;  
	int centerY = sizeY / 2;

	// Initialising the grid values, with height according to gaussian pulse
	for (int i = 0; i < sizeX; i++) {
		for (int j = 0; j < sizeY; j++) {

			float dx = i - centerX;
			float dy = j - centerY;
			float distanceSquared = dx * dx + dy * dy;
			float pulse = maxHeight * exp(-distanceSquared / (2 * pulseWidth * pulseWidth));

			grid[j][i][Height] = pulse;
			grid[j][i][DischargeX] = 0;
			grid[j][i][DischargeY] = 0;
			grid[j][i][Bathymetry] = 0;
		}
	}
}

SimulationGrid2D::~SimulationGrid2D()
{
	grid.clear();
}

std::vector<std::vector<std::array<float, 4>>>& SimulationGrid2D::GetSimulationGrid2D()
{
	return grid;
}

std::array<float, 4>& SimulationGrid2D::GetNode(int x, int y)
{
	return grid[y][x];
}

void SimulationGrid2D::SetValue(GridValues data, int x, int y, float newValue)
{
	grid[y][x][data] = newValue;
}

int SimulationGrid2D::GetSizeX()
{
	return sizeX;
}

int SimulationGrid2D::GetSizeY()
{
	return sizeY;
}
