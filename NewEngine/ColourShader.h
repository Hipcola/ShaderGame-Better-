/*
    Flat colour shader - Matthew Caldwell
*/

#ifndef _FLATCOLOUR_H_
#define _FLATCOLOUR_H_

#include"Shader.h"
#include<DirectXMath.h>
using namespace DirectX;
//#include<xnamath.h>

struct ShaderDataFC
{
	XMMATRIX* worldMat;
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
	float* camPos;
	float* lightPos;
	float* lightCol;
	int inputType;
};

class ColourShader : public Shader
{
    public:
        ColourShader( );
		virtual ~ColourShader( );
        void Load(ID3D11Device* d3dDevice);
		void preRender(ID3D11DeviceContext*, void*);
		ID3D11InputLayout* instancedLayout_;
		ID3D11InputLayout* nonInstancedNormLayout_;
		ID3D11InputLayout* nonInstancedLayout_;

	private:
		struct ConstantBufferType
		{
			XMMATRIX world;
			XMMATRIX view;
			XMMATRIX projection;
			XMFLOAT4 lightColour;
			XMFLOAT3 camPos;
			XMFLOAT3 lightPos;
		};
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3D11Buffer* constantBuffer;
};


#endif