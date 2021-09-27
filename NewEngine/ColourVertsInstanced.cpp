#include"ColourVertsInstanced.h"
#include"FlatColourInstanced.h"
ColourVertsInstanced::ColourVertsInstanced()
{
}

ColourVertsInstanced::~ColourVertsInstanced( )
{
}

/*void ColourVertsInstanced::Load(ID3D11Device* d3dDevice)
{
	RawVertsInstanced::Load(d3dDevice);
}*/
void ColourVertsInstanced::Update(float delta)
{
}

void ColourVertsInstanced::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	//XMMATRIX* viewMat = &((renderData*)data)->camera->GetViewMatrix();
    //viewMat = &XMMatrixTranspose(*viewMat );

	ShaderDataFCI d;
	d.viewMat = ((renderData*)data)->viewMat;
	d.projMat = ((renderData*)data)->projMat;
	d.worldMat = &XMLoadFloat4x4(&worldMat);
	((FlatColourInstanced*)&shaders[0])->preRender(d3dContext,&d);

	RawVertsInstanced::Render(d3dContext,data);
}