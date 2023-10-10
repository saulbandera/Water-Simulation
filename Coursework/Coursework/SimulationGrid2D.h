#pragma once
#include <array>
#include <vector>

class SimulationGrid2D
{

public:

	enum GridValues
	{
		Height = 0,
		DischargeX = 1,
		DischargeY = 2,
		Bathymetry = 3 // not used 
	};

	SimulationGrid2D(int nx, int ny);
	~SimulationGrid2D();

	// Get the simulation grid data structure
	std::vector<std::vector<std::array<float, 4>>>& GetSimulationGrid2D();

	// Get the array containing data for a specific grid node
	std::array<float, 4>& GetNode(int x, int y);

	// Used to set the grid values
	void SetValue(GridValues data, int x, int y, float newValue);

	// Get the size of the grid:
	int GetSizeX();
	int GetSizeY();

private:

	// Data structure used to store the 2D grid 
	std::vector<std::vector<std::array<float, 4>>> grid;

	// Size variables for the grid
	int sizeX;
	int sizeY;
	int resolution;

};
