#include "CorrectionShader.h"
#include <iostream>

CorrectionShader::CorrectionShader(ID3D11Device* dev, ID3D11DeviceContext* deviceCntxt, HWND hwnd, int gsize) : BaseShader(device, hwnd)
{
	gridSize = gsize;
	device = dev;
	deviceContext = deviceCntxt;
	initShader(L"default_vs.cso", L"corrector_step_ps.cso");
}

CorrectionShader::~CorrectionShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer) {
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	// Release the layout.
	if (layout) {
		layout->Release();
		layout = 0;
	}
	//Release base shader components
	BaseShader::~BaseShader();

}

void CorrectionShader::setSimulationParameters(float g_, float n_, float timeStepSize_, float cr_)
{
	gravity = g_;
	n = n_;
	timeStepSize = timeStepSize_;
	cr = cr_;
}


void CorrectionShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{

	// Load (+ compile) shader files
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
	device->CreateBuffer(&matrixBufferDesc, nullptr, &matrixBuffer);

	// Setup time buffer
	D3D11_BUFFER_DESC timeBufferDesc;
	timeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	timeBufferDesc.ByteWidth = sizeof(TimeBufferType);
	timeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeBufferDesc.MiscFlags = 0;
	timeBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&timeBufferDesc, nullptr, &timeBuffer);


	// Setup simulation parameter buffer
	D3D11_BUFFER_DESC simulationBufferDesc;
	simulationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	simulationBufferDesc.ByteWidth = sizeof(SimulationBufferType);
	simulationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	simulationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	simulationBufferDesc.MiscFlags = 0;
	simulationBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&simulationBufferDesc, nullptr, &simulationBuffer);


	// Create a texture sampler state for accessing the simulation grid data
	D3D11_SAMPLER_DESC samplerDesc;
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



void CorrectionShader::setShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, float DTDXDY, ID3D11ShaderResourceView* predictedGridTex, ID3D11ShaderResourceView* correctedGridTex, bool firstPass, SimulationGrid2D* predictedGrid, SimulationGrid2D * correctedGrid)
{
	HRESULT result; // to do: use this to check for errors
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Send matrix data
	MatrixBufferType* dataPtr;
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = XMMatrixTranspose(world);
	dataPtr->view = XMMatrixTranspose(view);
	dataPtr->projection = XMMatrixTranspose(projection);
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


	// Send time and grid size data
	TimeBufferType* timeDataPtr;
	deviceContext->Map(timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	timeDataPtr = (TimeBufferType*)mappedResource.pData;
	timeDataPtr->DTDXDY = DTDXDY;
	timeDataPtr->gridSizeX = gridSize;
	timeDataPtr->gridSizeY = gridSize;
	if (firstPass)
	{
		timeDataPtr->firstPass = 1.f;
	}
	else
	{
		timeDataPtr->firstPass = 0.f;
	}
	deviceContext->Unmap(timeBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &timeBuffer);



	// Send simulation parameters
	SimulationBufferType* simDataPtr;
	deviceContext->Map(simulationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	simDataPtr = (SimulationBufferType*)mappedResource.pData;
	simDataPtr->gravity = gravity;
	simDataPtr->n = n;
	simDataPtr->timeStepSize = timeStepSize;
	simDataPtr->cr = cr;

	deviceContext->Unmap(simulationBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &simulationBuffer);



	// Set render texture resource containing the predicted simulation grid data in the pixel shader.
	deviceContext->PSSetShaderResources(2, 1, &predictedGridTex);

	// Set render texture resource containing the corrected simulation grid data in the pixel shader.
	deviceContext->PSSetShaderResources(3, 1, &correctedGridTex);


	deviceContext->PSSetSamplers(0, 1, &sampleStateGrid);



}
