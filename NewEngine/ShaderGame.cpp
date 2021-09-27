/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    ShaderGame - Demonstrates rendering a 3D object (cube).
*/


#include"ShaderGame.h"
#include<xnamath.h>
#include<string>
#include<time.h>
#include <stdio.h>
#include <crtdbg.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11.h>
using namespace std;

struct VertexPos
{
    XMFLOAT3 pos;
    XMFLOAT2 tex0;
};

ShaderGame::ShaderGame( ) : effect_( 0 ), solidColorVS_( 0 ), solidColorPS_( 0 ), inputLayout_( 0 ),
                        colorMap_( 0 ), colorMapSampler_( 0 ), vertexBuffer_( 0 ), indexBuffer_( 0 ),
                        viewCB_( 0 ), projCB_( 0 ), worldCB_( 0 ), secondMap_( 0 )
{
	g_pSamplerLinear = NULL;
	g_pTextureRV = NULL;
}


ShaderGame::~ShaderGame( )
{

}


bool ShaderGame::LoadContent( )
{
    ID3DBlob* buffer = 0;
	bool compileResult = CompileD3DShader( "MultiTexture.fx", 0, "fx_5_0", &buffer );
    if( compileResult == false )
    {
        MessageBox( 0, "Error loading vertex shader!", "Compile Error", MB_OK );
        return false;
	}

    HRESULT d3dResult;
	d3dResult = D3DX11CreateEffectFromMemory( buffer->GetBufferPointer( ),
        buffer->GetBufferSize( ), 0, d3dDevice_, &effect_ );
    
    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Error creating the effect shader!" );

        if( buffer )
            buffer->Release( );

        return false;
    }
    /*d3dResult = d3dDevice_->CreateVertexShader( vsBuffer->GetBufferPointer( ),
        vsBuffer->GetBufferSize( ), 0, &solidColorVS_ );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Error creating the vertex shader!" );

        if( vsBuffer )
            vsBuffer->Release( );

        return false;
    }*/

    D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    unsigned int totalLayoutElements = ARRAYSIZE( solidColorLayout );

    ID3DX11EffectTechnique* multiTexTechnique;
    multiTexTechnique = effect_->GetTechniqueByName( "MultiTexture" );
    ID3DX11EffectPass* effectPass = multiTexTechnique->GetPassByIndex( 0 );

    D3DX11_PASS_SHADER_DESC passDesc;
    D3DX11_EFFECT_SHADER_DESC shaderDesc;
    effectPass->GetVertexShaderDesc( &passDesc );
    passDesc.pShaderVariable->GetShaderDesc( passDesc.ShaderIndex, &shaderDesc );

    d3dResult = d3dDevice_->CreateInputLayout( solidColorLayout, totalLayoutElements,
        shaderDesc.pBytecode, shaderDesc.BytecodeLength, &inputLayout_ );

    buffer->Release( );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Error creating the input layout!" );
        return false;
    }
	
	//PixelShader
	ID3DBlob* psBuffer = 0;
    compileResult = CompileD3DShader( "GS.fx", "PS_Main", "ps_5_0", &psBuffer );

    if( compileResult == false )
    {
        MessageBox( 0, "Error loading pixel shader!", "Compile Error", MB_OK );
        return false;
    }

    d3dResult = d3dDevice_->CreatePixelShader( psBuffer->GetBufferPointer( ),
        psBuffer->GetBufferSize( ), 0, &solidColorPS_ );

    psBuffer->Release( );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Error creating pixel shader!" );
        return false;
    }

    D3D11_BUFFER_DESC constDesc;
	ZeroMemory( &constDesc, sizeof( constDesc ) );
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.ByteWidth = sizeof( XMMATRIX );
	constDesc.Usage = D3D11_USAGE_DEFAULT;

	d3dResult = d3dDevice_->CreateBuffer( &constDesc, 0, &viewCB_ );

	if( FAILED( d3dResult ) )
    {
        return false;
    }

    d3dResult = d3dDevice_->CreateBuffer( &constDesc, 0, &projCB_ );

	if( FAILED( d3dResult ) )
    {
        return false;
    }

    d3dResult = d3dDevice_->CreateBuffer( &constDesc, 0, &worldCB_ );

	if( FAILED( d3dResult ) )
    {
        return false;
    } 

	VertexPos vertices[] =
    {
        { XMFLOAT3( -1.0f,  1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  1.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  1.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  1.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3(  1.0f, -1.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  1.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

        { XMFLOAT3( -1.0f, -1.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  1.0f, -1.0f,  1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  1.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  1.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
    };

	D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory( &vertexDesc, sizeof( vertexDesc ) );
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof( VertexPos ) * 24;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory( &resourceData, sizeof( resourceData ) );
    resourceData.pSysMem = vertices;

    d3dResult = d3dDevice_->CreateBuffer( &vertexDesc, &resourceData, &vertexBuffer_ );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Failed to create vertex buffer!" );
        return false;
    }

    WORD indices[] =
    {
        3,   1,  0,  2,  1,  3,
        6,   4,  5,  7,  4,  6,
        11,  9,  8, 10,  9, 11,
        14, 12, 13, 15, 12, 14,
        19, 17, 16, 18, 17, 19,
        22, 20, 21, 23, 20, 22
    };

    D3D11_BUFFER_DESC indexDesc;
    ZeroMemory( &indexDesc, sizeof( indexDesc ) );
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.ByteWidth = sizeof( WORD ) * 36;
    indexDesc.CPUAccessFlags = 0;
    resourceData.pSysMem = indices;

    d3dResult = d3dDevice_->CreateBuffer( &indexDesc, &resourceData, &indexBuffer_ );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Failed to create index buffer!" );
        return false;
    }

    d3dResult = D3DX11CreateShaderResourceViewFromFile( d3dDevice_,
        "decalMulti1.dds", 0, 0, &colorMap_, 0 );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Failed to load the texture image!" );
        return false;
    }


    d3dResult = D3DX11CreateShaderResourceViewFromFile( d3dDevice_,
        "decalMulti2.dds", 0, 0, &secondMap_, 0 );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Failed to load the second texture image!" );
        return false;
    }

    D3D11_SAMPLER_DESC colorMapDesc;
    ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ) );
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

    d3dResult = d3dDevice_->CreateSamplerState( &colorMapDesc, &colorMapSampler_ );

    if( FAILED( d3dResult ) )
    {
        DXTRACE_MSG( "Failed to create color map sampler state!" );
        return false;
    }

    projMatrix_ = XMMatrixPerspectiveFovLH( XM_PIDIV4, 800.0f / 600.0f, 0.01f, 1000.0f );
    projMatrix_ = XMMatrixTranspose( projMatrix_ );

	//FOR TEXTURES
	if (g_pTextureRV == NULL){
		D3DX11CreateShaderResourceViewFromFile( d3dDevice_, "Bark.dds", NULL, NULL, &g_pTextureRV, NULL );
	}
	if (g_pSamplerLinear == NULL){
		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		d3dDevice_->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
	}

    camera_.SetDistance( 12.0f, 1.0f, 2000.0f );

    return true;
}


void ShaderGame::UnloadContent( )
{
    if( colorMapSampler_ ) colorMapSampler_->Release( );
    if( colorMap_ ) colorMap_->Release( );
    if( solidColorVS_ ) solidColorVS_->Release( );
    if( solidColorPS_ ) solidColorPS_->Release( );
    if( inputLayout_ ) inputLayout_->Release( );
	if( g_pTextureRV ) g_pTextureRV->Release();
	if( g_pSamplerLinear ) g_pSamplerLinear->Release();
   // if( vertexBuffer_ ) vertexBuffer_->Release( );
   // if( indexBuffer_ ) indexBuffer_->Release( );
    if( viewCB_ ) viewCB_->Release( );
    if( projCB_ ) projCB_->Release( );
    if( worldCB_ ) worldCB_->Release( );
	//for (int i = 2;i>-1;i--){
	//	delete &tree_[i];
	//}
	//if (tree_) delete tree_;
    colorMapSampler_ = 0;
    colorMap_ = 0;
    solidColorVS_ = 0;
    solidColorPS_ = 0;
    inputLayout_ = 0;
 //   vertexBuffer_ = 0;
  //  indexBuffer_ = 0;
    viewCB_ = 0;
    projCB_ = 0;
    worldCB_ = 0;
}


void ShaderGame::Update( float dt )
{
	
	float yawDelta = 0.0f;
    float pitchDelta = 0.0f;

	keyboardDevice_->GetDeviceState( sizeof( keyboardKeys_ ), ( LPVOID )&keyboardKeys_ );

    // Button press event.
    if( GetAsyncKeyState( VK_ESCAPE ) )
	{ 
		PostQuitMessage( 0 );
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_E))
	{ 
		camera_.ApplyZoom( 0.005f );
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_Q ))
	{ 
		camera_.ApplyZoom( -0.005f );
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_W ) )
	{ 
		yawDelta = -0.001f;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_S ) )
	{ 
		yawDelta = 0.001f;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_A ) )
	{ 
		pitchDelta = -0.001f;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_D ) )
	{ 
		pitchDelta = 0.001f;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_M ) && !KEYDOWN(keyboardKeys_, DIK_M ) )
	{ 
		D3D11_RASTERIZER_DESC rasterDesc; 
		m_rasterState->GetDesc(&rasterDesc);
		if (rasterDesc.FillMode == D3D11_FILL_SOLID)
			rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
		else
			rasterDesc.FillMode = D3D11_FILL_SOLID;

		// Create the rasterizer state from the description we just filled out.
		d3dDevice_->CreateRasterizerState(&rasterDesc, &m_rasterState);
		// Now set the rasterizer state.
		d3dContext_->RSSetState(m_rasterState);
	}
	memcpy( prevKeyboardKeys_, keyboardKeys_, sizeof( keyboardKeys_ ) );

	camera_.ApplyRotation( yawDelta, pitchDelta );
}


void ShaderGame::Render( )
{
    if( d3dContext_ == 0 )
        return;

    float clearColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
    d3dContext_->ClearRenderTargetView( backBufferTarget_, clearColor );
    d3dContext_->ClearDepthStencilView( depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	d3dContext_->IASetInputLayout( inputLayout_ );

    d3dContext_->VSSetShader( solidColorVS_, 0, 0 );
    d3dContext_->PSSetShader( solidColorPS_, 0, 0 );
    
	XMMATRIX worldMat = XMMatrixIdentity( );
    worldMat = XMMatrixTranspose( worldMat );

    XMMATRIX viewMat = camera_.GetViewMatrix( );
    viewMat = XMMatrixTranspose( viewMat );

    d3dContext_->UpdateSubresource( worldCB_, 0, 0, &worldMat, 0, 0 );
    d3dContext_->UpdateSubresource( viewCB_, 0, 0, &viewMat, 0, 0 );
    d3dContext_->UpdateSubresource( projCB_, 0, 0, &projMatrix_, 0, 0 );

    d3dContext_->VSSetConstantBuffers( 0, 1, &worldCB_ );
    d3dContext_->VSSetConstantBuffers( 1, 1, &viewCB_ );
    d3dContext_->VSSetConstantBuffers( 2, 1, &projCB_ );
	d3dContext_->PSSetShaderResources( 0, 1, &g_pTextureRV );
    d3dContext_->PSSetSamplers( 0, 1, &g_pSamplerLinear );

    swapChain_->Present( 0, 0 );
}