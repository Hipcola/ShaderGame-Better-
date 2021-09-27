#include"Leaves.h"
#include"Texture.h"
#include"GlobalData.h"

Leaves::Leaves()
{
	leafSize = 0.01f;
}

Leaves::Leaves(float leafSize)
{
	this->leafSize = leafSize;
}

Leaves::~Leaves( )
{
	leafPositions.clear();
	leafRotPositions.clear();
}

void Leaves::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta,void* data)
{

}

void Leaves::AddLeaf(XMFLOAT3 v)
{
	leafPositions.push_back(v);
}

void Leaves::AddLeaf(XMFLOAT3 v, XMFLOAT4 pitch, XMFLOAT4 yaw, XMFLOAT4 roll)
{
	InstanceDataRot d;
	d.pos = v;
	d.pitch = XMFLOAT3(pitch.x,pitch.y,pitch.z);
	d.yaw = XMFLOAT3(yaw.x,yaw.y,yaw.z);
	d.roll = XMFLOAT3(roll.x,roll.y,roll.z);
	leafRotPositions.push_back(d);
	//leafPitch.push_back(XMFLOAT3(pitch.x,pitch.y,pitch.z));
	//leafYaw.push_back(XMFLOAT3(yaw.x,yaw.y,yaw.z));
	//leafRoll.push_back(XMFLOAT3(roll.x,roll.y,roll.z));
}

void Leaves::EmptyLeaves()
{
	leafPositions.clear();
	leafRotPositions.clear();
}

void Leaves::SetBasicLeafGeo(ID3D11Device* d3dDevice)
{
	//Setting quad
	VertexPosRVTex vertices[] =
    {
		{ XMFLOAT3( -leafSize/2, 0.f, 0.f ), XMFLOAT2( 1.f, 1.f )  },
		{ XMFLOAT3( -leafSize/2, leafSize*2, 0.f ), XMFLOAT2( 1.f, 0.001f ) },
		{ XMFLOAT3( leafSize/2, 0.f, 0.f ), XMFLOAT2( 0.001f, 1.f ) },
		{ XMFLOAT3( leafSize/2, leafSize*2, 0.f ), XMFLOAT2( 0.001f, 0.001f ) }
	};

	WORD indices[] =
    {
        2,1,0,1,2,3
    };

	RawVerts::Load(d3dDevice, 
		vertices, sizeof(VertexPosRVTex), 4, 
		indices, sizeof(WORD), 6, 
		NULL, NULL, 0);
	setPosition(XMFLOAT3(0.0f,0.0f,0.0f));
	setRotation(&XMMatrixRotationRollPitchYaw( 0.0f , 0.0f, 0.0f ));// 1.57079633f


}

void Leaves::Build(ID3D11Device* d3dDevice)
{
	if (leafRotPositions.size() > 0)
		RawVerts::Load(d3dDevice, 
			NULL, NULL, 0, 
			NULL, NULL, 0, 
			leafRotPositions.data(), sizeof(InstanceDataRot), leafRotPositions.size());
	else
		RawVerts::Load(d3dDevice, 
			NULL, NULL, 0, 
			NULL, NULL, 0, 
			leafPositions.data(), sizeof(InstanceData), leafPositions.size());
}

void Leaves::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	if (leafRotPositions.size() > 0)
	{
		ShaderDataT d;
		d.viewMat = ((RenderData*)data)->viewMat;
		d.projMat = ((RenderData*)data)->projMat;
		d.worldMat = &XMLoadFloat4x4(&worldMat);
		d.inputType = 2;
		((Texture*)&shaders[0])->preRender(d3dContext,&d);
		RawVerts::Render(d3dContext,data);
	}
	else if (leafPositions.size() > 0)
	{
		ShaderDataT d;
		d.viewMat = ((RenderData*)data)->viewMat;
		d.projMat = ((RenderData*)data)->projMat;
		d.worldMat = &XMLoadFloat4x4(&worldMat);
		d.inputType = 1;
		((Texture*)&shaders[0])->preRender(d3dContext,&d);
		RawVerts::Render(d3dContext,data);
	}
}