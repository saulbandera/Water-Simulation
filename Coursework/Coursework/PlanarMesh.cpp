#include "PlanarMesh.h"
#include <iostream>
PlanarMesh::PlanarMesh(ID3D11Device* ID3D11_device, ID3D11DeviceContext* device_context, int res)
{
	resolution = res;
	device = ID3D11_device;
	deviceContext = device_context;
	initBuffers(device);
}

// Release resources
PlanarMesh::~PlanarMesh()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}


void PlanarMesh::initBuffers(ID3D11Device* device)
{

	// Generate plane (including texture coordinates and normals)

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Number of vertices (x,z)
	UINT m = resolution;
	UINT n = resolution;

	// Size of grid/plane
	float width = 100;
	float depth = 100;

	vertexCount = m * n;
	UINT faceCnt = (m - 1) * (n - 1) * 6;

	// Create vertices
	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	vertices = new VertexType[vertexCount];
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;
			vertices[i * n + j].position = XMFLOAT3(x, 0.0f, z);

			vertices[i * n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertices[i * n + j].texture.x = j * du;
			vertices[i * n + j].texture.y = i * dv;


		}
	}

	// Set indices
	indexCount = faceCnt * 3;
	indices = new unsigned long[indexCount];
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			indices[k + 5] = i * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 3] = (i + 1) * n + j;
			indices[k + 2] = (i + 1) * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k] = (i + 1) * n + j + 1;
			k += 6;
		}
	}


	// Set up the description of the static vertex buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the buffers have been created and loaded
	delete[] indices;
	indices = 0;

}

int PlanarMesh::GetResolution()
{
	return resolution;
}




