#pragma once
#include "DXF.h"
#include "WaveShader.h"
#include "PlanarMesh.h"


class Water
{
public:
	Water(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, TextureManager* textureMgr, int gridSizeX);
	~Water();

	void render(XMMATRIX& camview, XMMATRIX& world, XMMATRIX& projection, Light* light, XMFLOAT3 camPos, float timeVar, ID3D11ShaderResourceView* gridTexture, bool toggleGerstner, bool toggleSWE);
	void GUI();

private:
	WaveShader* wavesShader;
	PlanarMesh* waveMesh;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	TextureManager* textureMgr;


	// Gerstner wave simulation parameters
	float wavelength; // length between wave crests
	float speed;      // speed at which the waves move
	float amplitude;  // height of the waves
	float steepness;  // steepness of the waves

};

