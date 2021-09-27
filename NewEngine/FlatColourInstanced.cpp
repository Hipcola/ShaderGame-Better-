#include"FlatColourInstanced.h"

FlatColourInstanced::FlatColourInstanced()
{

}

FlatColourInstanced::~FlatColourInstanced( )
{
    if( effect ) effect->Release( );
	if( inputLayout_ ) inputLayout_->Release( );
    effect = 0;
    inputLayout_ = 0;
}

void FlatColourInstanced::Load(ID3D11Device* d3dDevice)
{
	ID3DBlob* buffer = 0;

    bool compileResult = CompileD3DShader( "FlatColourInstanced.fx", 0, "fx_5_0", &buffer );

    if( compileResult == false )
    {
        //DXTRACE_MSG( "Error compiling the effect shader!" );
    }

	compileResult = CompileD3DEffect(d3dDevice, buffer, &effect);

	//INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	unsigned int totalLayoutElements = ARRAYSIZE( solidColorLayout );

	ID3DX11EffectTechnique* multiTexTechnique;
	multiTexTechnique = effect->GetTechniqueByName( "FlatColourInstanced" );
	ID3DX11EffectPass* effectPass = multiTexTechnique->GetPassByIndex( 0 );

	D3DX11_PASS_SHADER_DESC passDesc;
	D3DX11_EFFECT_SHADER_DESC shaderDesc;
	effectPass->GetVertexShaderDesc( &passDesc );
	passDesc.pShaderVariable->GetShaderDesc( passDesc.ShaderIndex, &shaderDesc );

	HRESULT a = d3dDevice->CreateInputLayout( solidColorLayout, totalLayoutElements,
		shaderDesc.pBytecode, shaderDesc.BytecodeLength, &inputLayout_ );

	buffer->Release( );
}

void FlatColourInstanced::preRender(ID3D11DeviceContext* d3dContext, void* data)
{
	d3dContext->IASetInputLayout( inputLayout_ );
	ShaderDataFCI* d = (ShaderDataFCI*)data;

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