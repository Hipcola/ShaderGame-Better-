#include"SkyShader.h"
#include"DDSTextureLoader.h"

SkyShader::SkyShader() : colorMap_( 0 ), colorMapSampler_( 0 )
{

}

SkyShader::~SkyShader( )
{
	if( colorMapSampler_ ) colorMapSampler_->Release( );
    if( colorMap_ ) colorMap_->Release( );
    if( effect ) effect->Release( );
	if( inputLayout_ ) inputLayout_->Release( );
    colorMapSampler_ = 0;
    colorMap_ = 0;
    effect = 0;
    inputLayout_ = 0;
}

void SkyShader::Load(ID3D11Device* d3dDevice)
{
	ID3DBlob* buffer = 0;

    bool compileResult = CompileD3DShader( L"SkyboxNEW.fx", 0, "fx_5_0", &buffer );

    if( compileResult == false )
    {
        //DXTRACE_MSG( "Error compiling the effect shader!" );
    }

	compileResult = CompileD3DEffect(d3dDevice, buffer, &effect);

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	unsigned int totalLayoutElements = ARRAYSIZE( solidColorLayout );

	ID3DX11EffectTechnique* multiTexTechnique;
	multiTexTechnique = effect->GetTechniqueByName( "RenderSkybox" );
	ID3DX11EffectPass* effectPass = multiTexTechnique->GetPassByIndex( 0 );

	D3DX11_PASS_SHADER_DESC passDesc;
	D3DX11_EFFECT_SHADER_DESC shaderDesc;
	effectPass->GetVertexShaderDesc( &passDesc );
	passDesc.pShaderVariable->GetShaderDesc( passDesc.ShaderIndex, &shaderDesc );

	d3dDevice->CreateInputLayout( solidColorLayout, totalLayoutElements,
		shaderDesc.pBytecode, shaderDesc.BytecodeLength, &inputLayout_ );

	buffer->Release( );

	CreateDDSTextureFromFile(d3dDevice, L"cubemap-evul.dds", nullptr, &colorMap_);
    //D3DX11CreateShaderResourceViewFromFile( d3dDevice,
    //    "cubemap-evul.dds", 0, 0, &colorMap_, 0 );
}

void SkyShader::preRender(ID3D11DeviceContext* d3dContext, void* data)
{
	d3dContext->IASetInputLayout( inputLayout_ );
	ShaderDataSB* d = (ShaderDataSB*)data;

	ID3DX11EffectShaderResourceVariable* colorMap;
    colorMap = effect->GetVariableByName( "g_EnvironmentMap" )->AsShaderResource( );
    colorMap->SetResource( colorMap_ );

	ID3DX11EffectMatrixVariable* g_pmViewProj;
    g_pmViewProj = effect->GetVariableByName( "g_mViewProjection" )->AsMatrix( );
    XMFLOAT4X4 fFakeView;
	XMMATRIX mFakeView;
    XMMATRIX mFakeViewProj;
	XMMATRIX mProj = *d->projMat;

	// Get the projection & view matrices from the camera class then fake em up
	
	//mFakeView = *d->viewMat;
	XMStoreFloat4x4(&fFakeView,*d->viewMat);
	fFakeView._41 = 0; // <_41, _42, _43> corresponds to distance vector, Skybox has "no distance"
	fFakeView._42 = 0;
	fFakeView._43 = 0;
	mFakeView = XMLoadFloat4x4(&fFakeView);
	mFakeViewProj = mFakeView * mProj;

	// Set effect variables
    g_pmViewProj->SetMatrix( ( float* )&mFakeViewProj );


	ID3DX11EffectScalarVariable* sunAlphaScalar;
	sunAlphaScalar = effect->GetVariableByName( "g_SunAlpha" )->AsScalar( );
	sunAlphaScalar->SetInt( *d->sunAlpha );

	ID3DX11EffectScalarVariable* sunThetaScalar;
	sunThetaScalar = effect->GetVariableByName( "g_SunTheta" )->AsScalar();
	sunThetaScalar->SetInt( *d->sunTheta );

	ID3DX11EffectScalarVariable* sunShineScalar;
	sunShineScalar = effect->GetVariableByName( "g_SunShininess" )->AsScalar();
	sunShineScalar->SetInt( *d->sunShine );

	ID3DX11EffectScalarVariable* sunStrengthScalar;
	sunStrengthScalar = effect->GetVariableByName( "g_SunStrength" )->AsScalar();
	sunStrengthScalar->SetInt( *d->sunSunStrength );


}