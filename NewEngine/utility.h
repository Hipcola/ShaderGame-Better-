#pragma once
#include<vector>
#include<sstream>
#include<d3d11.h>
//#include<D3DX11.h>

//#include<d3d11.h>
namespace Utility
{
	std::string convertFloat(float number);
	std::wstring convertFloatW(float number);
	std::string convertInt(int number);
	float convertString(std::string number);
	float convertString(std::string number, bool &wasValid);
	HRESULT CreateConstantBuffer(ID3D11Device* d3dDevice, 
		UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut );
	HRESULT CreateStructuredBuffer(ID3D11Device* d3dDevice, 
		UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut );
	HRESULT CreateDrawIndirectBuffer(ID3D11Device* d3dDevice, 
		UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut );
	HRESULT CreateBufferSRV(ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut );
	HRESULT CreateBufferUAV(ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut );
	ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11Buffer* pBuffer );
};