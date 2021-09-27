/*
    Render object - Matthew Caldwell
*/

#ifndef _RINSTANCEVERTS_H_
#define _RINSTANCEVERTS_H_

#include"RenderObject.h"
#include<xnamath.h>

struct VertexPosRVI
{
    XMFLOAT3 pos;
};

struct InstanceDataRVI
{
    XMFLOAT3 pos;
};

struct LoadDataRVI
{
    VertexPosRVI* verts;
	WORD* inds;
	InstanceDataRVI* insts;
	UINT numOfVerts,numOfInds,numOfInsts;
};

struct InstanceDataG
{
    XMFLOAT2 pos;
};


class RawVertsInstanced : public RenderObject
{
    public:
        RawVertsInstanced( );
		virtual ~RawVertsInstanced( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Load(ID3D11Device* d3dDevice, LoadDataRVI*);
		virtual void Update(float);
		//TODO. Templates for load function?
		void setPosition(XMFLOAT3);
		void setRotation(XMMATRIX*);
    protected:
		XMFLOAT3 position;
		XMFLOAT4X4 rotationMat;
		XMFLOAT4X4 worldMat;
		ID3D11Buffer* vertexBuffer_;
        ID3D11Buffer* indexBuffer_;
		ID3D11Buffer* instanceBuffer_;
		UINT instanceCount;
		UINT vertexCount;
		UINT indicesCount;
		
};
#endif