/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    DirectX 11 Base Class - Used as base class for all DirectX 11 demos in this book.
*/


#include"Dx11Base.h"
#include<D3Dcompiler.h>
//#include"GlobalData.h"


Dx11Base::Dx11Base( ) : driverType_( D3D_DRIVER_TYPE_NULL ), featureLevel_( D3D_FEATURE_LEVEL_11_0 ),
                                d3dDevice_( 0 ), d3dContext_( 0 ), swapChain_( 0 ), backBufferTarget_( 0 ),
                                directInput_( 0 ), keyboardDevice_( 0 ), mouseDevice_( 0 ),
                                depthTexture_( 0 ), depthStencilView_( 0 )
{

}


Dx11Base::~Dx11Base( )
{
    Shutdown( );
}


bool Dx11Base::Initialize( HINSTANCE hInstance, HWND hwnd )
{
    hInstance_ = hInstance;
    hwnd_ = hwnd;

    RECT dimensions;
    GetClientRect( hwnd, &dimensions );

    unsigned int width = dimensions.right - dimensions.left;
    unsigned int height = dimensions.bottom - dimensions.top;

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
    };

    unsigned int totalDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    unsigned int totalFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 100; // 0 = 60 fps
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    unsigned int creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result;
    unsigned int driver = 0;

    for( driver = 0; driver < totalDriverTypes; ++driver )
    {
        result = D3D11CreateDeviceAndSwapChain( 0, driverTypes[driver], 0, creationFlags,
                                                featureLevels, totalFeatureLevels,
                                                D3D11_SDK_VERSION, &swapChainDesc, &swapChain_,
                                                &d3dDevice_, &featureLevel_, &d3dContext_ );

        if( SUCCEEDED( result ) )
        {
            driverType_ = driverTypes[driver];
            break;
        }
    }

    if( FAILED( result ) )
    {
        //DXTRACE_MSG( "Failed to create the Direct3D device!" );
        return false;
    }

    ID3D11Texture2D* backBufferTexture;

    result = swapChain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&backBufferTexture );

    if( FAILED( result ) )
    {
        //DXTRACE_MSG( "Failed to get the swap chain back buffer!" );
        return false;
    }

    result = d3dDevice_->CreateRenderTargetView( backBufferTexture, 0, &backBufferTarget_ );

    if( backBufferTexture )
        backBufferTexture->Release( );

    if( FAILED( result ) )
    {
       // DXTRACE_MSG( "Failed to create the render target view!" );
        return false;
    }

    //d3dContext_->OMSetRenderTargets( 1, &backBufferTarget_, 0 );

 /*3D*/
	//*	
	D3D11_TEXTURE2D_DESC depthTexDesc;
    ZeroMemory( &depthTexDesc, sizeof( depthTexDesc ) );
    depthTexDesc.Width = width;
    depthTexDesc.Height = height;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    result = d3dDevice_->CreateTexture2D( &depthTexDesc, NULL, &depthTexture_ );

    if( FAILED( result ) )
    {
        //DXTRACE_MSG( "Failed to create the depth texture!" );
        return false;
    }

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof( descDSV ) );
    descDSV.Format = depthTexDesc.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    result = d3dDevice_->CreateDepthStencilView( depthTexture_, &descDSV, &depthStencilView_ );

    if( FAILED( result ) )
    {
        //DXTRACE_MSG( "Failed to create the depth stencil target view!" );
        return false;
    }

    d3dContext_->OMSetRenderTargets( 1, &backBufferTarget_, depthStencilView_ ); 

	//Rasterizer
	//*
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;//D3D11_CULL_NONE;//;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = /*D3D11_FILL_WIREFRAME;*/D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = d3dDevice_->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	d3dContext_->RSSetState(m_rasterState);
	//GlobalData::rasterizerState = m_rasterState;
	//*/

    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    d3dContext_->RSSetViewports( 1, &viewport );

	ZeroMemory( keyboardKeys_, sizeof( keyboardKeys_ ) );
    ZeroMemory( prevKeyboardKeys_, sizeof( prevKeyboardKeys_ ) );

    result = DirectInput8Create( hInstance_, DIRECTINPUT_VERSION, IID_IDirectInput8, ( void** )&directInput_, 0 );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = directInput_->CreateDevice( GUID_SysKeyboard, &keyboardDevice_, 0 );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = keyboardDevice_->SetDataFormat( &c_dfDIKeyboard );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = keyboardDevice_->SetCooperativeLevel( hwnd_, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = keyboardDevice_->Acquire( );

    if( FAILED( result ) )
    {
		//DXTRACE_MSG( "Failed to acquire keyboard!" );
        return false;
    }
	mousePosX_ = mousePosY_ = mouseWheel_ = 0;

    result = directInput_->CreateDevice( GUID_SysMouse, &mouseDevice_, 0 );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = mouseDevice_->SetDataFormat( &c_dfDIMouse );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = mouseDevice_->SetCooperativeLevel( hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );

    if( FAILED( result ) )
    { 
        return false;
    }

    result = mouseDevice_->Acquire( );

    if( FAILED( result ) )
    { 
		//DXTRACE_MSG( "Failed to acquire mouse!" );
        return false;
    }

    return LoadContent( );
}


bool Dx11Base::LoadContent( )
{
    // Override with demo specifics, if any...
    return true;
}


void Dx11Base::UnloadContent( )
{
    // Override with demo specifics, if any...
}


void Dx11Base::Shutdown( )
{
    UnloadContent( );

	if( depthTexture_ ) depthTexture_->Release( );
    if( depthStencilView_ ) depthStencilView_->Release( );
    if( backBufferTarget_ ) backBufferTarget_->Release( );
    if( swapChain_ ) swapChain_->Release( );
    if( d3dContext_ ) d3dContext_->Release( );
    if( d3dDevice_ ) d3dDevice_->Release( );    

    if( keyboardDevice_ )
    { 
        keyboardDevice_->Unacquire( );
        keyboardDevice_->Release( );
    }

	if( mouseDevice_ )
    {
        mouseDevice_->Unacquire( );
        mouseDevice_->Release( );
    }

    if( directInput_ ) directInput_->Release( );

	depthTexture_ = 0;
    depthStencilView_ = 0;
    backBufferTarget_ = 0;
    swapChain_ = 0;
    d3dContext_ = 0;
    d3dDevice_ = 0;
	keyboardDevice_ = 0;
	mouseDevice_ = 0;
    directInput_ = 0;
}