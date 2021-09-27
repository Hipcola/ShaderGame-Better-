#include"TexAnimVertsInstanced.h"
#include"AnimShaderInstanced.h"
TexAnimVertsInstanced::TexAnimVertsInstanced()
{
	animCounter = 0;
	time(&lastTime);
}

TexAnimVertsInstanced::~TexAnimVertsInstanced( )
{
}

/*void TexAnimVertsInstanced::Load(ID3D11Device* d3dDevice)
{
	RawVertsInstanced::Load(d3dDevice);
}*/
void TexAnimVertsInstanced::Update(float delta)
{
	time_t timeN;
	time(&timeN);
	double d = difftime(timeN,lastTime);
	if (d >= 0.1){
		if (animCounter == 3)
			animCounter = 0;
		else
			animCounter++;
		time(&lastTime);
	}
}

void TexAnimVertsInstanced::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	//XMMATRIX* viewMat = &((renderData*)data)->camera->GetViewMatrix();
    //viewMat = &XMMatrixTranspose(*viewMat );

	ShaderDataASI d;
	d.viewMat = ((renderData*)data)->viewMat;
	d.projMat = ((renderData*)data)->projMat;
	d.worldMat = &XMLoadFloat4x4(&worldMat);
	d.animCounter = &animCounter;
	((AnimShaderInstanced*)&shaders[0])->preRender(d3dContext,&d);

	RawVertsInstanced::Render(d3dContext,data);
}