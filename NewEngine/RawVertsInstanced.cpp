#include"RawVertsInstanced.h"
RawVertsInstanced::RawVertsInstanced()
{
	XMStoreFloat4x4(&rotationMat,XMMATRIX());
	XMStoreFloat4x4(&worldMat,XMMATRIX());
	position = XMFLOAT3();
	indicesCount = 0;
	instanceCount = 0;
}

RawVertsInstanced::~RawVertsInstanced( )
{
    if( vertexBuffer_ ) vertexBuffer_->Release( );
    if( indexBuffer_ ) indexBuffer_->Release( );
	if( instanceBuffer_) instanceBuffer_->Release( );
    vertexBuffer_ = 0;
    indexBuffer_ = 0;
	instanceBuffer_ = 0;
}

void RawVertsInstanced::setPosition(XMFLOAT3 p)
{
	position = p;
	XMMATRIX translationMat = XMMatrixTranslation( position.x, position.y, position.z );
	XMStoreFloat4x4(&worldMat,(XMLoadFloat4x4(&rotationMat) * translationMat));
}

void RawVertsInstanced::setRotation(XMMATRIX* r)
{
	XMStoreFloat4x4(&rotationMat,*r);
	XMMATRIX translationMat = XMMatrixTranslation( position.x, position.y, position.z );
    XMStoreFloat4x4(&worldMat,(*r * translationMat));
}

void RawVertsInstanced::Load(ID3D11Device* d3dDevice, LoadDataRVI* data)
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

	indicesCount = data->numOfInds;
    D3D11_BUFFER_DESC indexDesc;
    ZeroMemory( &indexDesc, sizeof( indexDesc ) );
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.ByteWidth = sizeof( WORD ) * data->numOfInds;
    indexDesc.CPUAccessFlags = 0;
    resourceData.pSysMem = data->inds;

    d3dDevice->CreateBuffer( &indexDesc, &resourceData, &indexBuffer_ );

	//delete [] indices;

	// Set up the description of the instance buffer.
	instanceCount = data->numOfInsts;
	D3D11_BUFFER_DESC instanceBufferDesc;
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(InstanceDataRVI) * data->numOfInsts;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	resourceData.pSysMem = data->insts;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;
	d3dDevice->CreateBuffer( &instanceBufferDesc, &resourceData, &instanceBuffer_);
}

void RawVertsInstanced::Update(float delta)
{
}


void RawVertsInstanced::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	unsigned int stride[2];
	unsigned int offset[2] = {0,0};
	stride[0] = sizeof(VertexPosRVI);
	stride[1] = sizeof(InstanceDataRVI);

	ID3D11Buffer* bufferPointers[2];
	bufferPointers[0] = vertexBuffer_;	
	bufferPointers[1] = instanceBuffer_;
    d3dContext->IASetVertexBuffers( 0, 2, bufferPointers, stride, offset );
    d3dContext->IASetIndexBuffer( indexBuffer_, DXGI_FORMAT_R16_UINT, 0 );
    d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    ID3DX11EffectTechnique* multiTexTechnique;
    multiTexTechnique = shaders[0].effect->GetTechniqueByName( effectName );

    D3DX11_TECHNIQUE_DESC techDesc;
    multiTexTechnique->GetDesc( &techDesc );

    for( unsigned int p = 0; p < techDesc.Passes; p++ )
    {
        ID3DX11EffectPass* pass = multiTexTechnique->GetPassByIndex( p );

        if( pass != 0 )
        {
            pass->Apply( 0, d3dContext );
			d3dContext->DrawIndexedInstanced(indicesCount,instanceCount,0,0,0);
        }
    }
}