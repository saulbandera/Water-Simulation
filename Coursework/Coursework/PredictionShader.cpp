#include "PredictionShader.h"
#include <iostream>

PredictionShader::PredictionShader(ID3D11Device* dev, ID3D11DeviceContext* deviceCntxt, HWND hwnd, int gsize) : BaseShader(device, hwnd)
{
	gridSize = gsize;
	device = dev;
	deviceContext = deviceCntxt;
	initShader(L"default_vs.cso", L"predictor_step_ps.cso");
}

PredictionShader::~PredictionShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer){
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	// Release the layout.
	if (layout){
		layout->Release();
		layout = 0;
	}
	//Release base shader components
	BaseShader::~BaseShader();

}

void PredictionShader::setSimulationParameters(float g_, float n_, float timeStepSize_, float cr_)
{
	gravity = g_;
	n = n_;
	timeStepSize = timeStepSize_;
	cr = cr_;
}


void PredictionShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
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




void PredictionShader::setShaderParameters(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, float DTDXDY, ID3D11ShaderResourceView* predictedGridTex, ID3D11ShaderResourceView* correctedGridTex, bool firstPass, SimulationGrid2D* predictedGrid, SimulationGrid2D* correctedGrid)
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
	if(firstPass)
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


	// Creating the 2D texture containing the predicted grid
	D3D11_TEXTURE2D_DESC desc2D;
	D3D11_SUBRESOURCE_DATA data2D;

	// Flatten the 2D grid to 1D so it can be stored in the 2D texture
	std::vector<std::vector<std::array<float, 4>>> grid2D = predictedGrid->GetSimulationGrid2D();
	std::vector<std::array<float, 4>> grid1D;

	for (auto& row : grid2D) {
		grid1D.insert(grid1D.end(), row.begin(), row.end());
	}
	// Set the 2D texture data
	data2D.pSysMem = grid1D.data();
	data2D.SysMemPitch = gridSize * sizeof(std::array<float, 4>);
	// Defining the 2D texture
	desc2D.Width = gridSize;
	desc2D.Height = gridSize;
	desc2D.MipLevels = 1;
	desc2D.ArraySize = 1;
	desc2D.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // R holds height, G holds x flux, B holds y flux
	desc2D.SampleDesc.Count = 1;
	desc2D.SampleDesc.Quality = 0;
	desc2D.Usage = D3D11_USAGE_DYNAMIC;
	desc2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc2D.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc2D.MiscFlags = 0;
	device->CreateTexture2D(&desc2D, &data2D, &predictedGridTexture2D);

	// Create shader resource view for 2D texture
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(predictedGridTexture2D, &SRVDesc, &predictedGridTexture2DView);




	// Creating the 2D texture containing the corrected grid
	std::vector<std::vector<std::array<float, 4>>> grid2Dc = correctedGrid->GetSimulationGrid2D();
	std::vector<std::array<float, 4>> grid1Dc;

	for (auto& row : grid2Dc) {
		grid1Dc.insert(grid1Dc.end(), row.begin(), row.end());
	}
	data2D.pSysMem = grid1Dc.data();
	data2D.SysMemPitch = gridSize * sizeof(std::array<float, 4>);
	desc2D.Width = gridSize;
	desc2D.Height = gridSize;
	desc2D.MipLevels = 1;
	desc2D.ArraySize = 1;
	desc2D.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc2D.SampleDesc.Count = 1;
	desc2D.SampleDesc.Quality = 0;
	desc2D.Usage = D3D11_USAGE_DYNAMIC;
	desc2D.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc2D.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc2D.MiscFlags = 0;
	device->CreateTexture2D(&desc2D, &data2D, &correctedGridTexture2D);

	SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(correctedGridTexture2D, &SRVDesc, &correctedGridTexture2DView);


	// Bind the 2D predicted grid texture to the pixel shader
	deviceContext->PSSetShaderResources(0, 1, &predictedGridTexture2DView);

	// Bind the 2D corrected grid texture to the pixel shader
	deviceContext->PSSetShaderResources(1, 1, &correctedGridTexture2DView);

	// Set render texture resource containing the predicted simulation grid data in the pixel shader.
	deviceContext->PSSetShaderResources(2, 1, &predictedGridTex);

	// Set render texture resource containing the corrected simulation grid data in the pixel shader.
	deviceContext->PSSetShaderResources(3, 1, &correctedGridTex);


	deviceContext->PSSetSamplers(0, 1, &sampleStateGrid);



}
