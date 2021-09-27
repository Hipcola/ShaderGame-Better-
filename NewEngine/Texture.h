/*
    Shader base - Matthew Caldwell
*/

#ifndef _TEX_H_
#define _TEX_H_

#include"Shader.h"
#include"DDSTextureLoader.h"
#include<DirectXMath.h>
using namespace DirectX;
//#include<xnamath.h>

struct ShaderDataT
{
	XMMATRIX* worldMat;
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
	int inputType;
};

class Texture : public Shader
{
    public:
        Texture( );
		virtual ~Texture( );
        void Load(ID3D11Device* d3dDevice);
		void fileNameLoad(ID3D11Device* d3dDevice, wchar_t* fName);
		void preRender(ID3D11DeviceContext*, void*);
		ID3D11ShaderResourceView* colorMap_;
		ID3D11SamplerState* colorMapSampler_;
		ID3D11InputLayout* instancedLayout_;
		ID3D11InputLayout* instancedRotLayout_;
		ID3D11InputLayout* nonInstancedLayout_;

		//ID3D11Buffer* constantBuffer;
		//ID3D11VertexShader* vertexShader;
		//ID3D11PixelShader* pixelShader;


	private:
		
		/*
		struct ConstantBufferType
		{
			XMMATRIX* _1;
			XMMATRIX* _2;
			XMMATRIX* _3;
		};
		*/

		wchar_t* texFileName;
};


#endif