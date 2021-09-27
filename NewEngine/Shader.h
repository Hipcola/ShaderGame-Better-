/*
    Shader base - Matthew Caldwell
*/

#ifndef _SHADER_H_
#define _SHADER_H_

#include<d3d11.h>
#include<d3dx11effect.h>
#include<D3Dcompiler.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

class Shader
{
    public:
        Shader( );
		virtual ~Shader( );
        virtual void Load(ID3D11Device* d3dDevice) = 0;
		//virtual void preRender(ID3D11DeviceContext*, void*);
		ID3DX11Effect* effect;
    protected:
		bool CompileD3DShader(LPWSTR filePath, LPCSTR entry, LPCSTR shaderModel, ID3DBlob** buffer );
		bool CompileD3DEffect(ID3D11Device* device, ID3DBlob* buffer, ID3DX11Effect** effect);
		bool CreateStructuredBuffer(ID3D11Device* device, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut );
};


#endif