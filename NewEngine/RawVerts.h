/*
    Render object - Matthew Caldwell
*/


#ifndef _RVERTS_H_
#define _RVERTS_H_

#include"RenderObject.h"
//#include<xnamath.h>
//#include<DirectXMath.h>

#define MAXVERT 60000//24//65535
//#define MAXINDS MAXVERT*6
#define MAXINST 40000

struct VertexPosRVTex
{
    XMFLOAT3 pos;
    XMFLOAT2 tex0;
};

struct VertexPosNormRV
{
    XMFLOAT3 pos;
	XMFLOAT3 norm;
};

struct VertexPosNormTexRV
{
    XMFLOAT3 pos;
	XMFLOAT3 norm;
	XMFLOAT2 tex0;
};

struct VertexPosRV
{
    XMFLOAT3 pos;
	//VertexPosRV () { pos.x = 0; pos.y = 0, pos.z = 0; }
	//VertexPosRV (float x, float y, float z) { pos.x = x; pos.y = y, pos.z = z; }
};

struct InstanceData
{
    XMFLOAT3 pos;
};

struct InstanceDataRot
{
    XMFLOAT3 pos;
	XMFLOAT3 pitch;
	XMFLOAT3 yaw;
	XMFLOAT3 roll;
};

class RawVerts : public RenderObject
{
    public:
        RawVerts( );
		virtual ~RawVerts( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void RenderRaw(ID3D11DeviceContext*);
		//virtual void RenderInstanced(ID3D11DeviceContext*, void*);

		//User can pass in null/0 and set specific buffers
		void Load(ID3D11Device* d3dDevice,
			void* verts, unsigned int vertbyteSize, unsigned int numOfVerts,
			void* indices, unsigned int indbyteSize, unsigned int numOfInds,
			void* instances, unsigned int InstbyteSize, unsigned int numOfInsts);

		void Load(ID3D11Device* d3dDevice, std::vector<std::pair<unsigned int, unsigned int>>* arry,
			void* verts, unsigned int vertbyteSize, unsigned int numOfVerts,
			unsigned short** indices, unsigned short numOfBuffs, unsigned int numOfInds,
			void* instances, unsigned int InstbyteSize, unsigned int numOfInsts);
		void Clear();
		void setFlatSurface(ID3D11Device* d3dDevice, XMFLOAT2 Size, XMFLOAT2 Tex, XMFLOAT3 Direction, UINT Columns, UINT Rows);
		void setDome(ID3D11Device* d3dDevice, UINT detail, float radius);
		void setPosition(XMFLOAT3);
		XMFLOAT3 getPosition();
		void setRotation(XMMATRIX*);
    protected:
		XMFLOAT3 position;
		XMFLOAT4X4 rotationMat;
		XMFLOAT4X4 worldMat;

		unsigned int numVBuffers;
		std::vector<ID3D11Buffer*> vertexBuffers;
		unsigned int numIBuffers;
        std::vector<ID3D11Buffer*> indexBuffers;
		unsigned int numInstBuffers;
		std::vector<ID3D11Buffer*> instanceBuffers;
		unsigned int* vertexCount;
		unsigned int* indicesCount;
		unsigned int* instanceCount;
		UINT stride;
		UINT instStride;
		bool isLineList;
		//bool isInstanced;

};
#endif