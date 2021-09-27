#include"ComputeShader.h"

ComputeShader::ComputeShader()
{
	CShader = NULL;
}

ComputeShader::~ComputeShader( )
{
	SAFE_RELEASE(CShader);
}

void ComputeShader::fileNameLoad(ID3D11Device* d3dDevice, LPWSTR fName)
{
	fileName = fName;
	Load(d3dDevice);
}

void ComputeShader::Load(ID3D11Device* d3dDevice)
{
	ID3DBlob* buffer = 0;

	//This means that really this class shouldn't exist...
	//Shader specific defines for the tree compute
	/*
	const D3D_SHADER_MACRO defines[] = 
    {
		"sides","4",
		NULL, NULL
    };
	*/
	//Placed define within shader

	char* CSTarget = (d3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)? "cs_5_0" : "cs_4_0";

	bool compileResult = CompileD3DShader( fileName , "CSMain", CSTarget, &buffer );
    if( compileResult == false )
    {
        //DXTRACE_MSG( "Error compiling the effect shader!" );
    }

	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader);
	buffer->Release( );
}


//Prehaps this should be merged into the load function
bool ComputeShader::CompileD3DComputeShader( ID3D11Device* device, ID3DBlob* buffer, ID3D11ComputeShader** computeTarget)
{
	HRESULT d3dResult;
	UINT a = buffer->GetBufferSize( );
	d3dResult = device->CreateComputeShader( buffer->GetBufferPointer( ),
        buffer->GetBufferSize( ), 0, *(&computeTarget) );

    if( FAILED( d3dResult ) )
    {
       //DXTRACE_MSG( "Error creating the compute shader!" );
		OutputDebugStringA("Error creating the compute shader!");
        return false;
    }
	return true;
}

void ComputeShader::preRender(ID3D11DeviceContext* d3dContext, void* data)
{	

}

HRESULT ComputeShader::CreateBufferSRV( ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut )
{
    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof(descBuf) );
    pBuffer->GetDesc( &descBuf );

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
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

HRESULT ComputeShader::CreateBufferUAV( ID3D11Device* d3dDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut )
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