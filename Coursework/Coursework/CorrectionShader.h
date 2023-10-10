#pragma once
#include "DXF.h"
#include "SimulationGrid2D.h"
using namespace DirectX;

class CorrectionShader : public BaseShader
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

	CorrectionShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int gridSize);
	~CorrectionShader();
	void setSimulationParameters(float gravity, float n, float timeStepSize, float cr);

	void setShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, float DTDXDY, ID3D11ShaderResourceView* predictedGridTex, ID3D11ShaderResourceView* correctedGridTex, bool firstPass, SimulationGrid2D* predictedGrid, SimulationGrid2D* correctedGrid);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11DeviceContext* deviceContext;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* simulationBuffer;

	ID3D11SamplerState* sampleStateGrid;


	int gridSize;
	float gravity;
	float n;
	float timeStepSize;
	float cr;

};

