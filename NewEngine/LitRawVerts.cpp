#include"LitRawVerts.h"
#include"ColourShader.h"
#include"TexturePhongShader.h"
//#include <mmsystem.h>
LitRawVerts::LitRawVerts()
{

}

LitRawVerts::~LitRawVerts( )
{

}


void LitRawVerts::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta,void* data)
{

}

void LitRawVerts::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	ShaderDataTP d;
	d.viewMat = ((RenderData*)data)->viewMat;
	d.projMat = ((RenderData*)data)->projMat;
	d.worldMat = &XMLoadFloat4x4(&worldMat);
	XMFLOAT4* camP = ((RenderData*)data)->camPos;
	float camInP [] = {camP->x, camP->y, camP->z};
	d.camPos = camInP;
	if (instanceCount > 0 )
		d.inputType = 1;
	else
		d.inputType = 0;
	float lightCol [] = {lightColour.x,lightColour.y,lightColour.z,lightColour.w};
	d.lightCol = lightCol;
	float lightPos [] = {lightPosition.x,lightPosition.y,lightPosition.z};
	d.lightPos = lightPos;
	((/*TexturePhongShader*/ColourShader*)&shaders[0])->preRender(d3dContext,&d);
	RawVerts::RenderRaw(d3dContext);
}