/**
* \class Plane Mesh
*
* \brief Simple plane mesh object
*
* Inherits from Base Mesh, Builds a simple plane with texture coordinates and normals.
* Provided resolution values deteremines the subdivisions of the plane.
* Builds a plane from unit quads.
*
* \author Paul Robertson
*/


#ifndef _PLANEMESH_H_
#define _PLANEMESH_H_

#include "BaseMesh.h"
class Terrain;
class PlaneMesh : public BaseMesh
{

public:
	/** \brief Initialises and builds a plane mesh
	*
	* Can specify resolution of plane, this deteremines how many subdivisions of the plane.
	* @param device is the renderer device
	* @param device context is the renderer device context
	* @param resolution is a int for subdivision of the plane. The number of unit quad on each axis. Default is 100.
	*/
	PlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 100);
	~PlaneMesh();


	// Used to update the vertices of the plane mesh with the amplitude data from wave simulation
	// device - renderer device
	// resolution - an int for subdivision of the plane
	void setVertices(ID3D11Device* device, int resolution, ID3D11DeviceContext* deviceContext, VertexType* vertices);

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;

	VertexType* vertices;

};

#endif