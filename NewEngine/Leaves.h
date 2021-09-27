/*
    Render object - Matthew Caldwell
*/

#ifndef _LEAVES_H_
#define _LEAVES_H_

#include"RawVerts.h"
#include<vector>
using namespace std;

class Leaves : public RawVerts
{
    public:
        Leaves( );
		Leaves(float leafSize);
		virtual ~Leaves( );
		void SetBasicLeafGeo(ID3D11Device* d3dDevice);
		void AddLeaf(XMFLOAT3 v);
		void AddLeaf(XMFLOAT3 v, XMFLOAT4 pitch, XMFLOAT4 yaw, XMFLOAT4 roll);
		void EmptyLeaves();
		void Build(ID3D11Device* d3dDevice);
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float, void*);
	private:
		float leafSize;
		vector<XMFLOAT3> leafPositions;
		vector<InstanceDataRot> leafRotPositions;
		D3D11_RASTERIZER_DESC rasterDescBackFaceCull;
		D3D11_RASTERIZER_DESC rasterDescFullDraw;
		//vector<XMFLOAT3> leafPitch;
		//vector<XMFLOAT3> leafYaw;
		//vector<XMFLOAT3> leafRoll;

};
#endif