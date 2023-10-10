#ifndef _PLANARMESH_H_
#define _PLANARMESH_H_
#include "BaseMesh.h"

class PlanarMesh : public BaseMesh
{
public:

	// Initialises and builds a plane mesh
	// The resolution of the plane can be specified, this determines the vertex count of the plane.
	PlanarMesh(ID3D11Device* device, ID3D11DeviceContext* device_context, int resolution);
	~PlanarMesh();

	int GetResolution();


protected:
	void initBuffers(ID3D11Device* device) override;

	ID3D11DeviceContext* deviceContext;
	ID3D11Device* device;

	int resolution;
	VertexType* vertices;
	unsigned long* indices;

};

#endif