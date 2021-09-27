#include"Dome.h"
#include"SkyShader.h"
//#include <mmsystem.h>
Dome::Dome()
{
}

Dome::~Dome( )
{
}



void Dome::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data)
{
}

void Dome::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	ShaderDataSB d;
	d.viewMat = ((RenderData*)data)->viewMat;
	d.projMat = ((RenderData*)data)->projMat;
	float f = 1.38f;
	d.sunAlpha = &f;
	float f2 = 1.09f;
	d.sunTheta = &f2;
	float f3 = 84.0f;
	d.sunShine = &f3;
	float f4 = 12.0f;
	d.sunSunStrength = &f4;
	//TODO Casting Test. Look at other classes. Do we need to cast?
	//Go to the shader class and add a prerender?
	((SkyShader*)&shaders[0])->preRender(d3dContext,&d);

	RawVerts::Render(d3dContext,data);
}