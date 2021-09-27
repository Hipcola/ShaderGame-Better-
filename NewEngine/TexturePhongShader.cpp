#include"TexturePhongShader.h"
#include"DDSTextureLoader.h"

TexturePhongShader::TexturePhongShader() : map_( 0 ), sampler_( 0 )
{

}

TexturePhongShader::~TexturePhongShader( )
{
	if( nonInstancedLayout_ ) nonInstancedLayout_->Release( );
	if( instancedLayout_ ) instancedLayout_->Release( );
	if( sampler_ ) sampler_->Release( );
    if( map_ ) map_->Release( );
	instancedLayout_ = 0;
	bufferLayout_ = 0;
    nonInstancedLayout_ = 0;
    sampler_ = 0;
    map_ = 0;
}

void TexturePhongShader::Load(ID3D11Device* d3dDevice)
{
	ID3DBlob* buffer = 0;

    bool compileResult = CompileD3DShader( L"TexturedPhong.fx", 0, "fx_5_0", &buffer );

    if( compileResult == false )
    {
        //DXTRACE_MSG( "Error compiling the effect shader!" );
    }

	compileResult = CompileD3DEffect(d3dDevice, buffer, &effect);

	//INPUT LAYOUTS
	D3D11_INPUT_ELEMENT_DESC NoInstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	D3D11_INPUT_ELEMENT_DESC InstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	 // buffer
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	unsigned int NoInstNrmElements = ARRAYSIZE( NoInstanceLayout );
	unsigned int InstElements = ARRAYSIZE( InstanceLayout );

	ID3DX11EffectTechnique* TPTechnique;
	TPTechnique = effect->GetTechniqueByName( "TexturePhong" );
	ID3DX11EffectPass* effectPass = TPTechnique->GetPassByIndex( 0 );

	ID3DX11EffectTechnique* TPITechnique;
	TPITechnique = effect->GetTechniqueByName( "TexturePhongInstanced" );
	ID3DX11EffectPass* effectPass2 = TPITechnique->GetPassByIndex( 0 );

	ID3DX11EffectTechnique* TPBTechnique;
	TPBTechnique = effect->GetTechniqueByName( "TexturePhongBuffer" );
	ID3DX11EffectPass* effectPass3 = TPBTechnique->GetPassByIndex( 0 );

	D3DX11_PASS_SHADER_DESC passDesc;
	D3DX11_EFFECT_SHADER_DESC shaderDesc;
	effectPass->GetVertexShaderDesc( &passDesc );
	passDesc.pShaderVariable->GetShaderDesc( passDesc.ShaderIndex, &shaderDesc );

	D3DX11_PASS_SHADER_DESC passDesc2;
	D3DX11_EFFECT_SHADER_DESC shaderDesc2;
	effectPass2->GetVertexShaderDesc( &passDesc2 );
	passDesc2.pShaderVariable->GetShaderDesc( passDesc2.ShaderIndex, &shaderDesc2 );

	D3DX11_PASS_SHADER_DESC passDesc3;
	D3DX11_EFFECT_SHADER_DESC shaderDesc3;
	effectPass3->GetVertexShaderDesc( &passDesc3 );
	passDesc3.pShaderVariable->GetShaderDesc( passDesc3.ShaderIndex, &shaderDesc3 );

	d3dDevice->CreateInputLayout( NoInstanceLayout, NoInstNrmElements,
		shaderDesc.pBytecode, shaderDesc.BytecodeLength, &nonInstancedLayout_);

	d3dDevice->CreateInputLayout( InstanceLayout, InstElements,
		shaderDesc2.pBytecode, shaderDesc2.BytecodeLength, &instancedLayout_);

    d3dDevice->CreateInputLayout( layout, 1,
			shaderDesc3.pBytecode, shaderDesc3.BytecodeLength, &bufferLayout_ );
	
	buffer->Release( );

	//Getting map

	HRESULT a;
	CreateDDSTextureFromFile(d3dDevice, L"bark.dds", nullptr, &map_);
	//D3DX11CreateShaderResourceViewFromFile( d3dDevice, "bark.dds", 0, 0, &map_, &a );

	//building sampler
	D3D11_SAMPLER_DESC colorMapDesc;
    ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ) );
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

    d3dDevice->CreateSamplerState( &colorMapDesc, &sampler_ );
}

void TexturePhongShader::preRender(ID3D11DeviceContext* d3dContext, void* data)
{	
	ShaderDataTP* d = (ShaderDataTP*)data;
	if (d->inputType == 0)
		d3dContext->IASetInputLayout( nonInstancedLayout_ );
	else if (d->inputType == 1)
		d3dContext->IASetInputLayout( instancedLayout_ );
	else if (d->inputType == 2)
	{
		ID3DX11EffectShaderResourceVariable* bufferData;
		bufferData = effect->GetVariableByName( "gVertexBuffer" )->AsShaderResource();
		bufferData->SetResource( d->buffer );
		d3dContext->IASetInputLayout( NULL );
	}

	ID3DX11EffectMatrixVariable* worldMatrix;
    worldMatrix = effect->GetVariableByName( "worldMatrix" )->AsMatrix( );
	worldMatrix->SetMatrix( ( float* )d->worldMat);

	ID3DX11EffectShaderResourceVariable* map;
    map = effect->GetVariableByName( "map" )->AsShaderResource( );
    map->SetResource( map_ );

	ID3DX11EffectSamplerVariable* sampler;
    sampler = effect->GetVariableByName( "sampler_" )->AsSampler( );
    sampler->SetSampler( 0, sampler_ );

	ID3DX11EffectVectorVariable* camVec;
	camVec = effect->GetVariableByName( "camPos" )->AsVector( );
	camVec->SetFloatVector( d->camPos );

	ID3DX11EffectVectorVariable* lightVec;
	lightVec = effect->GetVariableByName( "lightPos" )->AsVector( );
	lightVec->SetFloatVector( d->lightPos );

	ID3DX11EffectVectorVariable* lightCVec;
	lightCVec = effect->GetVariableByName( "lightColour" )->AsVector( );
	lightCVec->SetFloatVector( d->lightCol);

    ID3DX11EffectMatrixVariable* viewMatrix;
    viewMatrix = effect->GetVariableByName( "viewMatrix" )->AsMatrix( );
    viewMatrix->SetMatrix( ( float* )d->viewMat );

    ID3DX11EffectMatrixVariable* projMatrix;
    projMatrix = effect->GetVariableByName( "projMatrix" )->AsMatrix( );
    projMatrix->SetMatrix( ( float* )d->projMat );

    /*ID3DX11EffectConstantBuffer* projMatrix;
    projMatrix = effect->GetVariableByName( "projMatrix" )->AsMatrix( );
    projMatrix->SetMatrix( ( float* )d->projMat );*/
}