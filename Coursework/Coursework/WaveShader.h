#pragma once
#include "DXF.h"
#include "BaseShader.h"

class WaveShader : public BaseShader
{

	struct LightBufferType {
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 position;
		XMFLOAT4 specularPower;
		XMFLOAT4 specular;
		XMFLOAT4 direction;
	};

	struct CameraBufferType{
		XMFLOAT3 position;
		float toggleGerstner;
		XMFLOAT2A direction;
	};

	struct TimeBufferType {
		float time;
		float speed;
		float amplitude;
		float steepness;
		// the following xmflaot contains the value of Pi and the wavelength 
		XMFLOAT2A PI_and_Wavelength;
	};



public:
	WaveShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int gridSizeX, ID3D11ShaderResourceView* texture);
	~WaveShader();
	void setGersnterWavesParameters(float speed, float amplitude, float wavelength, float steepness);
	void setShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Light* light, XMFLOAT3 camPos, bool toggleGerstner, ID3D11ShaderResourceView* gridTexture2DView, bool toggleSWE, float timeVar);


private: 
	void initShader(const wchar_t* cs, const wchar_t* ps);


	ID3D11ShaderResourceView* waterTexture;
	ID3D11DeviceContext* deviceContext;

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* timeBuffer;


	//! [SWE] sampler state object for accessing grid in the pixel shader
	ID3D11SamplerState* sampleStateGrid;
	int gridSizeX;



	//[gerstner]
	// variables to change water appearance
	float wavelength; // amount of waves
	float speed;      // speed at which the waves move
	float amplitude;  // height of the waves
	float steepness;  // steepness of the waves

};

