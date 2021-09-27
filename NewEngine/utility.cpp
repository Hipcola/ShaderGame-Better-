#include "utility.h"
namespace Utility
{
	std::string convertFloat(float number)
	{
		std::stringstream ss;//create a stringstream
		ss << number;//add number to the stream
		return ss.str();//return a string with the contents of the stream
	}

	std::wstring convertFloatW(float number)
	{
		std::wstringstream ss;//create a stringstream
		ss << number;//add number to the stream
		return ss.str();//return a string with the contents of the stream
	}

	std::string convertInt(int number)
	{
		std::stringstream ss;//create a stringstream
		ss << number;//add number to the stream
		return ss.str();//return a string with the contents of the stream
	}

	float convertString(std::string number)
	{
		std::stringstream ss(number);//create a stringstream
		float output;
		ss >> output;//add number to the stream
		return output;
	}

	float convertString(std::string number, bool &wasValid)
	{
		std::stringstream ss(number);//create a stringstream
		float output;
		ss >> output;//add number to the stream
		if(ss.fail())
			wasValid = false;
		else
			wasValid = true;
		return output;
	}

	HRESULT CreateStructuredBuffer(ID3D11Device* d3dDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut )
	{
		*ppBufOut = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = uElementSize * uCount;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = uElementSize;

		if ( pInitData )
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return d3dDevice->CreateBuffer( &desc, &InitData, ppBufOut );
		} else
			return d3dDevice->CreateBuffer( &desc, NULL, ppBufOut );
	}

	HRESULT CreateDrawIndirectBuffer(ID3D11Device* d3dDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut )
	{
		*ppBufOut = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = uElementSize * uCount;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		desc.StructureByteStride = uElementSize;

		if ( pInitData )
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return d3dDevice->CreateBuffer( &desc, &InitData, ppBufOut );
		} else
			return d3dDevice->CreateBuffer( &desc, NULL, ppBufOut );
	}

	HRESULT CreateVertexBuffer(ID3D11Device* d3dDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut )
	{
		*ppBufOut = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = uElementSize * uCount;
		//desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		//desc.StructureByteStride = uElementSize;
		desc.CPUAccessFlags = 0;

		if ( pInitData )
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return d3dDevice->CreateBuffer( &desc, &InitData, ppBufOut );
		} else
			return d3dDevice->CreateBuffer( &desc, NULL, ppBufOut );
	}

	HRESULT CreateIndexBuffer(ID3D11Device* d3dDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut )
	{
		*ppBufOut = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;// | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = uElementSize * uCount;
		//desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		//desc.StructureByteStride = uElementSize;
		desc.CPUAccessFlags = 0;

		if ( pInitData )
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return d3dDevice->CreateBuffer( &desc, &InitData, ppBufOut );
		} else
			return d3dDevice->CreateBuffer( &desc, NULL, ppBufOut );
	}

	HRESULT CreateBufferSRV(ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut )
	{
		D3D11_BUFFER_DESC descBuf;
		ZeroMemory( &descBuf, sizeof(descBuf) );
		pBuffer->GetDesc( &descBuf );

		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		//desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.BufferEx.FirstElement = 0;

		if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
		{
			// This is a Raw Buffer

			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
		} else
		if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		} else
		{
			return E_INVALIDARG;
		}

		return d3dDevice->CreateShaderResourceView( pBuffer, &desc, ppSRVOut );
	}

	HRESULT CreateBufferUAV(ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut )
	{
		D3D11_BUFFER_DESC descBuf;
		ZeroMemory( &descBuf, sizeof(descBuf) );
		pBuffer->GetDesc( &descBuf );
        
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;

		if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
		{
			// This is a Raw Buffer

			desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
			desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			desc.Buffer.NumElements = descBuf.ByteWidth / 4; 
		} else
		if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride; 
		} else
		{
			return E_INVALIDARG;
		}
    
		return d3dDevice->CreateUnorderedAccessView( pBuffer, &desc, ppUAVOut );
	}

	ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11Buffer* pBuffer )
	{
		ID3D11Buffer* debugbuf = NULL;

		D3D11_BUFFER_DESC desc;
		ZeroMemory( &desc, sizeof(desc) );
		pBuffer->GetDesc( &desc );
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		if ( SUCCEEDED(d3dDevice->CreateBuffer(&desc, NULL, &debugbuf)) )
		{
			d3dContext->CopyResource( debugbuf, pBuffer );
		}

		return debugbuf;
	}
}
