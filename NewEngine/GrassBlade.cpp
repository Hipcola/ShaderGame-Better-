#include"GrassBlade.h"
#include"GrassShader.h"
GrassBlade::GrassBlade()
{
}

GrassBlade::~GrassBlade( )
{
}

void GrassBlade::Load(ID3D11Device* d3dDevice, LoadDataG* data)
{
	vertexCount = data->numOfVerts;
    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory( &vertexDesc, sizeof( vertexDesc ) );
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof( VertexPosRVI ) * data->numOfVerts;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory( &resourceData, sizeof( resourceData ) );
	resourceData.pSysMem = data->verts;

    d3dDevice->CreateBuffer( &vertexDesc, &resourceData, &vertexBuffer_ );

	//delete [] vertices;

	// Set up the description of the instance buffer.
	instanceCount = data->numOfInsts;
	D3D11_BUFFER_DESC instanceBufferDesc;
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(InstanceDataG) * data->numOfInsts;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	resourceData.pSysMem = data->insts;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;
	d3dDevice->CreateBuffer( &instanceBufferDesc, &resourceData, &instanceBuffer_);
}

