/*
    Texture with phong lighting shader - Matthew Caldwell
*/

#ifndef _TEXPHONG_H_
#define _TEXPHONG_H_

#include"Shader.h"
#include<DirectXMath.h>
using namespace DirectX;

struct ShaderDataTP
{
	XMMATRIX* worldMat;
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
	float* camPos;
	float* lightPos;
	float* lightCol;
	int inputType;
	ID3D11ShaderResourceView* buffer;
};

class TexturePhongShader : public Shader
{
    public:
        TexturePhongShader( );
		virtual ~TexturePhongShader( );
        void Load(ID3D11Device* d3dDevice);
		void preRender(ID3D11DeviceContext*, void*);
		ID3D11InputLayout* instancedLayout_;
		ID3D11InputLayout* nonInstancedLayout_;
		ID3D11InputLayout* bufferLayout_;
		ID3D11ShaderResourceView* map_;
        ID3D11SamplerState* sampler_;
};


#endif