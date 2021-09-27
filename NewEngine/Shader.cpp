#include"Shader.h"

Shader::Shader()
{
	effect = NULL;
}

Shader::~Shader( )
{
	SAFE_RELEASE(effect);
}

bool Shader::CompileD3DShader( LPWSTR filePath, LPCSTR entry, LPCSTR shaderModel, ID3DBlob** buffer )
{
    
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3DCOMPILE_DEBUG;
	#endif

    ID3DBlob* errorBuffer = 0;
    HRESULT result;

    result = D3DCompileFromFile( filePath, 0, 0, entry, shaderModel,
        shaderFlags, 0, buffer, &errorBuffer );

    if( FAILED( result ) )
    {
        if( errorBuffer != 0 )
        {
            OutputDebugStringA( ( char* )errorBuffer->GetBufferPointer( ) );
            errorBuffer->Release( );
        }

        return false;
    }
    
    if( errorBuffer != 0 ){
		OutputDebugStringA( ( char* )errorBuffer->GetBufferPointer( ) );
        errorBuffer->Release( );
	}
    return true;
}

bool Shader::CompileD3DEffect(ID3D11Device* d3dDevice, ID3DBlob* buffer, ID3DX11Effect** effect)
{
	HRESULT d3dResult;

    d3dResult = D3DX11CreateEffectFromMemory( buffer->GetBufferPointer( ),
        buffer->GetBufferSize( ), 0, d3dDevice, effect );
    
    if( FAILED( d3dResult ) )
    {
//        DXTRACE_MSG( "Error creating the effect shader!" );
        return false;
    }
	return true;
}


bool Shader::CreateStructuredBuffer(ID3D11Device* d3dDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut )
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