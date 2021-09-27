#include"Texture.h"

Texture::Texture()// : colorMap_( 0 ), colorMapSampler_( 0 ),secondMap_( 0 )
{

}

Texture::~Texture( )
{
	//if( colorMapSampler_ ) colorMapSampler_->Release( );
    //if( colorMap_ ) colorMap_->Release( );
    if( effect ) effect->Release( );
	if( nonInstancedLayout_ ) nonInstancedLayout_->Release( );
	if( instancedLayout_ ) instancedLayout_->Release( );
	if( instancedRotLayout_ ) instancedLayout_->Release( );
    //colorMapSampler_ = 0;
    //colorMap_ = 0;
    effect = 0;
    instancedLayout_ = 0;
    nonInstancedLayout_ = 0;
	instancedRotLayout_ = 0;
}

void Texture::fileNameLoad(ID3D11Device* d3dDevice, wchar_t* fName)
{
	texFileName = fName;
	Load(d3dDevice);
}

void Texture::Load(ID3D11Device* d3dDevice)
{
	/*
	ID3DBlob* buffer = 0;
	D3DReadFileToBlob(L"TextureVector.cso", &buffer);
	d3dDevice->CreateVertexShader(buffer->GetBufferPointer(),buffer->GetBufferSize(), NULL,&vertexShader);
	buffer->Release( );
	D3DReadFileToBlob(L"TexturePixel.cso", &buffer);
	d3dDevice->CreatePixelShader(buffer->GetBufferPointer(),buffer->GetBufferSize(), NULL,&pixelShader);
	buffer->Release( );
	D3D11_INPUT_ELEMENT_DESC NoInstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	D3D11_INPUT_ELEMENT_DESC InstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	D3D11_INPUT_ELEMENT_DESC InstanceRotLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "inRowOne", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "inRowTwo", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "inRowThree", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	unsigned int NoInstNrmElements = ARRAYSIZE( NoInstanceLayout );
	unsigned int InstElements = ARRAYSIZE( InstanceLayout );
	unsigned int InstRotElements = ARRAYSIZE( InstanceRotLayout );
	ID3DX11EffectTechnique* TPTechnique;
	TPTechnique = effect->GetTechniqueByName( "Texture" );
	ID3DX11EffectPass* effectPass = TPTechnique->GetPassByIndex( 0 );
	ID3DX11EffectTechnique* TPITechnique;
	TPITechnique = effect->GetTechniqueByName( "TextureInstanced" );
	ID3DX11EffectPass* effectPass2 = TPITechnique->GetPassByIndex( 0 );
	ID3DX11EffectTechnique* TPIRTechnique;
	TPIRTechnique = effect->GetTechniqueByName( "TextureRotatedInstanced" );
	ID3DX11EffectPass* effectPass3 = TPIRTechnique->GetPassByIndex( 0 );
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
	d3dDevice->CreateInputLayout( InstanceRotLayout, InstRotElements,
		shaderDesc3.pBytecode, shaderDesc3.BytecodeLength, &instancedRotLayout_);
	CreateDDSTextureFromFile(d3dDevice, texFileName, nullptr, &colorMap_);
    D3D11_SAMPLER_DESC colorMapDesc;
    ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ) );
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
    d3dDevice->CreateSamplerState( &colorMapDesc, &colorMapSampler_ );
	D3D11_BUFFER_DESC constantBufferDesc;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	d3dDevice->CreateBuffer(&constantBufferDesc, NULL, &constantBuffer);
	*/

	//*
	ID3DBlob* buffer = 0;
    bool compileResult = CompileD3DShader( L"Texture.fx", 0, "fx_5_0", &buffer );
    if( compileResult == false )
    {
        //DXTRACE_MSG( "Error compiling the effect shader!" );
    }
	compileResult = CompileD3DEffect(d3dDevice, buffer, &effect);
	//INPUT LAYOUTS
	D3D11_INPUT_ELEMENT_DESC NoInstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	D3D11_INPUT_ELEMENT_DESC InstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	D3D11_INPUT_ELEMENT_DESC InstanceRotLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD" , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "inRowOne", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "inRowTwo", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "inRowThree", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	unsigned int NoInstNrmElements = ARRAYSIZE( NoInstanceLayout );
	unsigned int InstElements = ARRAYSIZE( InstanceLayout );
	unsigned int InstRotElements = ARRAYSIZE( InstanceRotLayout );
	ID3DX11EffectTechnique* TPTechnique;
	TPTechnique = effect->GetTechniqueByName( "Texture" );
	ID3DX11EffectPass* effectPass = TPTechnique->GetPassByIndex( 0 );
	ID3DX11EffectTechnique* TPITechnique;
	TPITechnique = effect->GetTechniqueByName( "TextureInstanced" );
	ID3DX11EffectPass* effectPass2 = TPITechnique->GetPassByIndex( 0 );
	ID3DX11EffectTechnique* TPIRTechnique;
	TPIRTechnique = effect->GetTechniqueByName( "TextureRotatedInstanced" );
	ID3DX11EffectPass* effectPass3 = TPIRTechnique->GetPassByIndex( 0 );
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
	d3dDevice->CreateInputLayout( InstanceRotLayout, InstRotElements,
		shaderDesc3.pBytecode, shaderDesc3.BytecodeLength, &instancedRotLayout_);
	buffer->Release( );
	//HRESULT a;
	CreateDDSTextureFromFile(d3dDevice, texFileName, nullptr, &colorMap_);
    //a = D3DX11CreateShaderResourceViewFromFile( d3dDevice, texFileName, 0, 0, &colorMap_, 0 );
    D3D11_SAMPLER_DESC colorMapDesc;
    ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ) );
    colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
    d3dDevice->CreateSamplerState( &colorMapDesc, &colorMapSampler_ );
	//*/
}

void Texture::preRender(ID3D11DeviceContext* d3dContext, void* data)
{
	/*
	ShaderDataT* d = (ShaderDataT*)data;
	if (d->inputType == 0)
		d3dContext->IASetInputLayout( nonInstancedLayout_ );
	else if (d->inputType == 1)
		d3dContext->IASetInputLayout( instancedLayout_ );
	else if (d->inputType == 2)
		d3dContext->IASetInputLayout( instancedRotLayout_ );
	ConstantBufferType* dataPtr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (ConstantBufferType*)mappedResource.pData;
	dataPtr->_1 = d->projMat;
	dataPtr->_2 = d->viewMat;
	dataPtr->_3 = d->worldMat;
    d3dContext->Unmap(constantBuffer, 0);
    d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	d3dContext->PSSetShaderResources(0, 1, &colorMap_);
    d3dContext->VSSetShader(vertexShader, NULL, 0);
    d3dContext->PSSetShader(pixelShader, NULL, 0);
	d3dContext->PSSetSamplers(0, 1, &colorMapSampler_);
	*/
	ShaderDataT* d = (ShaderDataT*)data;
	if (d->inputType == 0)
		d3dContext->IASetInputLayout( nonInstancedLayout_ );
	else if (d->inputType == 1)
		d3dContext->IASetInputLayout( instancedLayout_ );
	else if (d->inputType == 2)
		d3dContext->IASetInputLayout( instancedRotLayout_ );

	ID3DX11EffectShaderResourceVariable* colorMap;
    colorMap = effect->GetVariableByName( "colorMap" )->AsShaderResource( );
    colorMap->SetResource( colorMap_ );

	ID3DX11EffectSamplerVariable* colorMapSampler;
    colorMapSampler = effect->GetVariableByName( "colorSampler" )->AsSampler( );
    colorMapSampler->SetSampler( 0, colorMapSampler_ );

	ID3DX11EffectMatrixVariable* worldMatrix;
    worldMatrix = effect->GetVariableByName( "worldMatrix" )->AsMatrix( );
	worldMatrix->SetMatrix( ( float* )d->worldMat);

    ID3DX11EffectMatrixVariable* viewMatrix;
    viewMatrix = effect->GetVariableByName( "viewMatrix" )->AsMatrix( );
    viewMatrix->SetMatrix( ( float* )d->viewMat);

    ID3DX11EffectMatrixVariable* projMatrix;
    projMatrix = effect->GetVariableByName( "projMatrix" )->AsMatrix( );
    projMatrix->SetMatrix( ( float* )d->projMat );
}