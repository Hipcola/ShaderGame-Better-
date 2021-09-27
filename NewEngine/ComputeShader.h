/*
    Compute shader - Matthew Caldwell
*/

#ifndef _COMPUTE_H_
#define _COMPUTE_H_

#include"Shader.h"
#include<windows.h>
//#include<xnamath.h>
#include<DirectXMath.h>

class ComputeShader : public Shader
{
    public:
        ComputeShader( );
		virtual ~ComputeShader( );
		//TODO
		//Do we want this? Should the effects shaders have this? Don't all shaders have such unique implementations?
		void fileNameLoad(ID3D11Device* d3dDevice, LPWSTR fName);
        void Load(ID3D11Device* d3dDevice);
		void preRender(ID3D11DeviceContext*, void*);
	protected:
		bool CompileD3DComputeShader( ID3D11Device* device, ID3DBlob* buffer, ID3D11ComputeShader** computeTarget);
		ID3D11ComputeShader* CShader;
		LPWSTR fileName;
		HRESULT CreateStructuredBuffer(ID3D11Device* d3dDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut );
		HRESULT CreateBufferSRV(ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut );
		HRESULT CreateBufferUAV(ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut );
};

#endif