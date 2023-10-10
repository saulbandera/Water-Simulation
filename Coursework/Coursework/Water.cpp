#include "Water.h"

Water::Water(ID3D11Device* device, ID3D11DeviceContext* deviceCtxt, HWND hwnd, TextureManager* texMgr, int gSizeX)
{
	deviceContext = deviceCtxt;
	textureMgr = texMgr;
	textureMgr->loadTexture(L"waves", L"res/waves.png");

	// initialise water surface mesh and shader used to render it
	waveMesh = new PlanarMesh(device, deviceContext, gSizeX);
	wavesShader = new WaveShader(device, deviceContext, hwnd, gSizeX, textureMgr->getTexture(L"waves"));

	// initial values for Gerstner waves
	speed = 3.f;
	amplitude = 0.3f;
	steepness = 0.8f;
	wavelength = 20.0f;
}

Water::~Water()
{
	if (wavesShader) {
		delete wavesShader;
	}
	if (waveMesh) {
		delete waveMesh;
	}
}


void Water::render(XMMATRIX& camView, XMMATRIX& world, XMMATRIX& projection, Light* light, XMFLOAT3 camPos, float timeVar, ID3D11ShaderResourceView* gridTexture, bool toggleGerstner, bool toggleSWE)
{

	// Render wave mesh
	waveMesh->sendData(deviceContext);
	if(toggleGerstner && !toggleSWE)
	{
		wavesShader->setGersnterWavesParameters(speed, amplitude, wavelength, steepness);
	}

	wavesShader->setShaderParameters(world, camView, projection, light, camPos, toggleGerstner, gridTexture, toggleSWE, timeVar);
	wavesShader->render(deviceContext, waveMesh->GetIndexCount());
}


void Water::GUI()
{
	// User interface settings for the waves
	if (ImGui::CollapsingHeader("Water Settings")) {
		ImGui::SliderFloat(" Speed", &speed, 0.0f, 100.0f);
		ImGui::SliderFloat(" Amplitude", &amplitude, 0.01f, 3.0f);
		ImGui::SliderFloat(" Steepness", &steepness, 0.0f, 1.0f);
		ImGui::SliderFloat(" Wavelength", &wavelength, 1.0f, 100.f);


		if (ImGui::Button(" Water Default Values", ImVec2(170, 20))) {
			speed = 3.f;
			amplitude = 0.3f;
			steepness = 0.9f;
			wavelength = 20.0f;
		}
	}
}
