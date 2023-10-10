#include "WaveShader.h"


WaveShader::WaveShader(ID3D11Device* dev, ID3D11DeviceContext* deviceCtxt, HWND hwnd, int gSizeX, ID3D11ShaderResourceView* text) : BaseShader(dev, hwnd)
{
	waterTexture = text;
	gridSizeX = gSizeX;
	device = dev;
	deviceContext = deviceCtxt;
	initShader(L"wave_vs.cso", L"wave_ps.cso");


	wavelength = 0.f; // length between wave crests
	speed = 0.f;      // speed at which the waves move
	amplitude = 0.f;  // height of the waves
	steepness = 0.f;  // steepness of the waves

}

WaveShader::~WaveShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	// Release the camera constant buffer.
	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();

}

void WaveShader::setGersnterWavesParameters(float speed_, float amplitude_, float wavelength_, float steepness_)
{
	 wavelength = wavelength_; 
	 speed = speed_;      
	 amplitude = amplitude_;  
	 steepness = steepness_;  

}



void WaveShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup matrix buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Setup camera buffer
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Setup light buffer
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	//Setup time Buffer
	D3D11_BUFFER_DESC timeBufferDesc;
	timeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	timeBufferDesc.ByteWidth = sizeof(TimeBufferType);
	timeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeBufferDesc.MiscFlags = 0;
	timeBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&timeBufferDesc, NULL, &timeBuffer);


	// Create a texture sampler state description
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &sampleState);


	// Create a texture sampler state for accessing the simulation grid data
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &sampleStateGrid);


}


void WaveShader::setShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Light* light, XMFLOAT3 camPos, bool toggleGerstner, ID3D11ShaderResourceView* gridTexture2DView, bool toggleSWE, float timeVar)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Send matrix buffer
	MatrixBufferType* dataPtr;
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = XMMatrixTranspose(world);;
	dataPtr->view = XMMatrixTranspose(view);;
	dataPtr->projection = XMMatrixTranspose(projection);
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Send camera position and time
	CameraBufferType* camDataPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camDataPtr = (CameraBufferType*)mappedResource.pData;
	camDataPtr->position = camPos;
	if(toggleSWE)
	{
		camDataPtr->toggleGerstner = 3;
	}
	else {
		camDataPtr->toggleGerstner = toggleGerstner;
	}
	camDataPtr->direction = XMFLOAT2A(1, 0.5f); // not used for now
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);

	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->position  = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0.0f);
	lightPtr->specularPower.x = light->getSpecularPower();
	lightPtr->specular = light->getSpecularColour();
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->diffuse = light->getDiffuseColour();;  
	// Setting the direction of the light to point towards the camera
	lightPtr->direction = XMFLOAT4(camPos.x - light->getPosition().x, -70, camPos.z - light->getPosition().z, 0.0f);
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);



	// send time and wave data to Vertex Shader 
	TimeBufferType* timePtr;
	deviceContext->Map(timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	timePtr = (TimeBufferType*)mappedResource.pData;
	timePtr->time = timeVar; // advancing time to move the water 
	timePtr->amplitude = amplitude;
	timePtr->PI_and_Wavelength.x = atan(1.f) * 4.f; // value of pi 
	timePtr->PI_and_Wavelength.y = wavelength;  // wavelength
	timePtr->steepness = steepness;
	timePtr->speed = speed;
	deviceContext->Unmap(timeBuffer, 0);
	deviceContext->VSSetConstantBuffers(2, 1, &timeBuffer);



	// Set water surface texture (image) resource in the pixel shader
	deviceContext->PSSetShaderResources(0, 1, &waterTexture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);

	// Set render texture resource containing the simulation grid data in the vertex shader for manipulation of the mesh
	deviceContext->VSSetShaderResources(0, 1, &gridTexture2DView);
	deviceContext->VSSetSamplers(0, 1, &sampleStateGrid);

}

