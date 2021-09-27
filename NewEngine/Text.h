/*
   Text shader - Matthew Caldwell
*/

#ifndef _TEXT_H_
#define _TEXT_H_

#include"Shader.h"
#include<DirectXMath.h>
using namespace DirectX;

#include<fstream>

struct ShaderDataText
{
	XMMATRIX* worldMat;
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
	XMFLOAT4* colour;
	UINT* indCount;
};

class Text : public Shader
{
    public:
        Text( );
		virtual ~Text( );
        void Load(ID3D11Device* d3dDevice);
		void fileNameLoad(ID3D11Device* d3dDevice, char* fName);
		void preRender(ID3D11DeviceContext*, void*);
		ID3D11ShaderResourceView* shaderTexture_;
		ID3D11SamplerState* colorMapSampler_;
		ID3D11InputLayout* layout_;

		void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	private:
		char* texFileName;
		struct ConstantBufferType
		{
			XMMATRIX world;
			XMMATRIX view;
			XMMATRIX projection;
		};

		struct PixelBufferType
		{
			XMFLOAT4 pixelColor;
		};
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_pixelShader;
		ID3D11InputLayout* m_layout;
		ID3D11Buffer* m_constantBuffer;
		ID3D11SamplerState* m_sampleState;
		ID3D11Buffer* m_pixelBuffer;
		ID3D11ShaderResourceView* texture;
};


#endif