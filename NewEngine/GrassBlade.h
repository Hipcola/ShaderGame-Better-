/*
    Render object - Matthew Caldwell
*/

#ifndef _GRASSBLADE_H_
#define _GRASSBLADE_H_

#include"RawVertsInstanced.h"

struct VertexPosGB
{
    XMFLOAT3 pos;
	XMFLOAT2 tex0;
};

struct LoadDataG
{
    VertexPosGB* verts;
	InstanceDataG* insts;
	UINT numOfVerts,numOfInsts;
};

class GrassBlade : public RawVertsInstanced
{
    public:
        GrassBlade( );
		virtual ~GrassBlade( );
		virtual void Load(ID3D11Device* d3dDevice, LoadDataG*);
};
#endif