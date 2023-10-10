#include "InstanceShader.h"

#include "utility.h"
#include "tracelog.h"

InstanceShader::InstanceShader(Device *device, HWND hwnd, bool init)
	: DefaultShader(device, hwnd) {
	if (init) {
		initShader(L"shaders/instance_vs.cso", L"shaders/instance_vs_depth.cso");
	}
}

InstanceShader::~InstanceShader() {
	RELEASE_IF_NOT_NULL(instanceBuffer);
}

void InstanceShader::initShader(const wchar_t *vs, const wchar_t *dvs) {
	loadVertexShader(vs);
	loadDepthShader(dvs);
	pixelShader = getDefaultPixelShader(this);

	initDefaultBuffers();
	addDiffuseSampler();
	addShadowSampler();
}

void InstanceShader::loadVertexShader(const wchar_t *vs) {
	// Create the vertex input layout description.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	
	BaseShader::loadVertexShader(vs, polygonLayout, ARR_LEN(polygonLayout));
}

void InstanceShader::renderInstanceInternal(Device *device, DeviceContext *ctx, MMesh &mesh, void *idata, size_t itypeSize, uint icount) {
	// -- INIT BUFFERS --------------------------------------------------------
	RELEASE_IF_NOT_NULL(instanceBuffer);
	D3D11_BUFFER_DESC ibufDesc{};
	D3D11_SUBRESOURCE_DATA instData{};

	// Set up the description of the instance buffer.
	ibufDesc.Usage = D3D11_USAGE_DEFAULT;
	ibufDesc.ByteWidth = uint(itypeSize * icount);
	ibufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// Give the subresource structure a pointer to the instance data.
	instData.pSysMem = idata;

	// Create the instance buffer.
	device->CreateBuffer(&ibufDesc, &instData, &instanceBuffer);

	// -- SEND DATA -----------------------------------------------------------

	uint strides[2]{}, offsets[2]{};

	ID3D11Buffer *bufferPtr[2]{};

	bufferPtr[0] = mesh.getVertexBuffer();
	bufferPtr[1] = instanceBuffer;

	// Set vertex buffer stride and offset.
	strides[0] = sizeof(MMesh::PubVertexType);
	strides[1] = (uint)itypeSize;

	// Set the buffer offsets.
	offsets[0] = offsets[1] = 0;

	ctx->IASetVertexBuffers(0, 2, bufferPtr, strides, offsets);
	ctx->IASetIndexBuffer(mesh.getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// -- RENDER --------------------------------------------------------------

	// Set the vertex input layout.
	ctx->IASetInputLayout(layout);

	// Set the vertex and pixel shaders that will be used to render.
	if (isDepth) {
		ctx->VSSetShader(vertexDepthShader, NULL, 0);
		ctx->PSSetShader(NULL, NULL, 0);
	}
	else {
		ctx->VSSetShader(vertexShader, NULL, 0);
		ctx->PSSetShader(pixelShader, NULL, 0);
	}
	ctx->CSSetShader(NULL, NULL, 0);

	// if Hull shader is not null then set HS and DS
	if (hullShader) {
		ctx->HSSetShader(hullShader, NULL, 0);
		ctx->DSSetShader(domainShader, NULL, 0);
	}
	else {
		ctx->HSSetShader(NULL, NULL, 0);
		ctx->DSSetShader(NULL, NULL, 0);
	}

	// if geometry shader is not null then set GS
	if (geometryShader) {
		ctx->GSSetShader(geometryShader, NULL, 0);
	}
	else {
		ctx->GSSetShader(NULL, NULL, 0);
	}

	if (isOmni) {
		ctx->GSSetShader(omniDepthGSShader, NULL, 0);
	}

	// Render the triangle.
	ctx->DrawIndexedInstanced(mesh.getIndexCount(), icount, 0, 0, 0);

	// unbind shader resources
	TextureType *nullTEX[LIGHTS_COUNT + 1] = { 0 };
	ctx->PSSetShaderResources(0, LIGHTS_COUNT + 1, nullTEX);
}
