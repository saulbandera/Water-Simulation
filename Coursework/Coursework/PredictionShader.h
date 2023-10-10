#pragma once
#include "DXF.h"
#include "SimulationGrid2D.h"
using namespace DirectX;

class PredictionShader : public BaseShader
{

	struct TimeBufferType {
		float DTDXDY;
		float gridSizeX;
		float gridSizeY;
		float firstPass;
	};


	struct SimulationBufferType
	{
		float gravity;
		float n;
		float timeStepSize;
		float cr;
	};


	struct CameraBufferType {
		XMFLOAT3 position;
		float time;
		XMFLOAT2A direction;
	};


public:

	PredictionShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int gridSize);
	~PredictionShader();
	void setSimulationParameters(float gravity, float n, float timeStepSize, float cr);

	void setShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, float DTDXDY, ID3D11ShaderResourceView* predictedGridTex, ID3D11ShaderResourceView* correctedGridTex, bool firstPass, SimulationGrid2D* predictedGrid, SimulationGrid2D* correctedGrid);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11DeviceContext* deviceContext;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* simulationBuffer;



	// 2D textures used to send the simulation grid to the GPU
	ID3D11Texture2D* predictedGridTexture2D;
	ID3D11Texture2D* correctedGridTexture2D;

	// Shader resource views for the 2D grid textures
	ID3D11ShaderResourceView* predictedGridTexture2DView;
	ID3D11ShaderResourceView* correctedGridTexture2DView;


	ID3D11SamplerState* sampleStateUVText;
	ID3D11SamplerState* sampleStateGrid;


	int gridSize;
	float gravity;
	float n;
	float timeStepSize;
	float cr;

};

