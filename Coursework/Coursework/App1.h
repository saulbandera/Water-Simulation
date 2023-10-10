// Application.h
#ifndef _APP1_H
#define _APP1_H
#include "DXF.h"
#include <Timer.h>
#include "Water.h"
#include "PredictionShader.h"
#include "CorrectionShader.h"
class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);
	bool frame();
	void initLight();

protected:

	bool render();
	void gui();

private:

	void predictionStep(XMMATRIX world, XMMATRIX view, XMMATRIX proj);
	void correctionStep(XMMATRIX world, XMMATRIX view, XMMATRIX proj);
	void App1::trackFrameRate();

	// Time related variables 
	float timeVar;
	float simulationTime;
	Timer time;


	// Shallow water equation simulation grids used to store height and fluxes x and y
	SimulationGrid2D* predictedGrid;
	SimulationGrid2D* correctedGrid;

	// Shallow water equation simulation parameters:
	int gridSizeX;
	float stepSizeX;
	int gridSizeY;
	float stepSizeY;
	float gravity;
	float n;
	float timeStepSize;
	float cr;
	float DTDXDY;
	float spatialStepSize;

	// Objects used for the simulation render passes
	OrthoMesh* orthoMesh;
	RenderTexture* predictionGridRTA;
	RenderTexture* predictionGridRTB;
	RenderTexture* correctionGridRTA;
	RenderTexture* correctionGridRTB;
	ID3D11RenderTargetView* renderTargetsA[2];
	ID3D11RenderTargetView* renderTargetsB[2];
	D3D11_VIEWPORT viewport;

	PredictionShader* predictionShader;
	CorrectionShader* correctionShader;

	bool firstPass;
	int  counter = 0;

	// Scene lights
	Light* light;  

	// 3D Scene Models 
	Water* water;

	// Rendering toggles
	bool renderWater = true;
	bool toggleGerstner = false;
	bool toggleSWE = false;
};

#endif