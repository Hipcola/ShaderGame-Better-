/*
    Shader base - Matthew Caldwell
*/


#ifndef _SKYSHADER_H_
#define _SKYSHADER_H_

#include"Shader.h"
#include<DirectXMath.h>
using namespace DirectX;
//#include<xnamath.h>

struct ShaderDataSB
{
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
	float* sunAlpha;
	float* sunTheta;
	float* sunShine;
	float* sunSunStrength;
};

class SkyShader : public Shader
{
    public:
        SkyShader( );
		virtual ~SkyShader( );
        void Load(ID3D11Device* d3dDevice);
		void preRender(ID3D11DeviceContext*, void*);
		ID3D11ShaderResourceView* colorMap_;
        ID3D11SamplerState* colorMapSampler_;
		ID3D11InputLayout* inputLayout_;
};
#endif