#include"ColourShader.h"

ColourShader::ColourShader()
{

}

ColourShader::~ColourShader( )
{
	if( nonInstancedLayout_ ) nonInstancedLayout_->Release( );
	if( instancedLayout_ ) instancedLayout_->Release( );
	instancedLayout_ = 0;
    nonInstancedLayout_ = 0;
}

void ColourShader::Load(ID3D11Device* d3dDevice)
{
	ID3DBlob* buffer = 0;
	D3DReadFileToBlob(L"ColourVector.cso", &buffer);
	d3dDevice->CreateVertexShader(buffer->GetBufferPointer(),buffer->GetBufferSize(), NULL,&vertexShader);
	buffer->Release( );
	D3DReadFileToBlob(L"ColourPixel.cso", &buffer);
	d3dDevice->CreatePixelShader(buffer->GetBufferPointer(),buffer->GetBufferSize(), NULL,&pixelShader);
	D3D11_INPUT_ELEMENT_DESC NoInstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3D11_INPUT_ELEMENT_DESC NoInstanceNormalsLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3D11_INPUT_ELEMENT_DESC InstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	unsigned int NoInstElements =	 ARRAYSIZE( NoInstanceLayout );
	unsigned int NoInstNrmElements = ARRAYSIZE( NoInstanceNormalsLayout );
	unsigned int InstElements =		 ARRAYSIZE( InstanceLayout );

	d3dDevice->CreateInputLayout( NoInstanceLayout, NoInstElements,
		buffer->GetBufferPointer(), buffer->GetBufferSize(), &nonInstancedLayout_);
	d3dDevice->CreateInputLayout( NoInstanceNormalsLayout, NoInstNrmElements,
		buffer->GetBufferPointer(), buffer->GetBufferSize(), &nonInstancedNormLayout_);
	d3dDevice->CreateInputLayout( InstanceLayout, InstElements,
		buffer->GetBufferPointer(), buffer->GetBufferSize(), &instancedLayout_);
	buffer->Release();

	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(unsigned int)*4;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	HRESULT test = d3dDevice->CreateBuffer(&bufferDesc, NULL, &constantBuffer);
	/*
	ID3DBlob* buffer = 0;
    bool compileResult = CompileD3DShader( L"Colour.fx", 0, "fx_5_0", &buffer );
    if( compileResult == false )
    {
       // DXTRACE_MSG( "Error compiling the effect shader!" );
    }
	compileResult = CompileD3DEffect(d3dDevice, buffer, &effect);
	//INPUT LAYOUTS
	//We create three input layouts, for objects that are instanced and those that arn't.
	D3D11_INPUT_ELEMENT_DESC NoInstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3D11_INPUT_ELEMENT_DESC NoInstanceNormalsLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3D11_INPUT_ELEMENT_DESC InstanceLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "inPos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	unsigned int NoInstElements = ARRAYSIZE( NoInstanceLayout );
	unsigned int NoInstNrmElements = ARRAYSIZE( NoInstanceNormalsLayout );
	unsigned int InstElements = ARRAYSIZE( InstanceLayout );
	ID3DX11EffectTechnique* FcTechnique;
	FcTechnique = effect->GetTechniqueByName( "FlatColour" );
	ID3DX11EffectPass* effectPass = FcTechnique->GetPassByIndex( 0 );
	ID3DX11EffectTechnique* CLTechnique;
	CLTechnique = effect->GetTechniqueByName( "Colour" );
	ID3DX11EffectPass* effectPass2 = CLTechnique->GetPassByIndex( 0 );
	ID3DX11EffectTechnique* IFcTechnique;
	IFcTechnique = effect->GetTechniqueByName( "FlatColourInstanced" );
	ID3DX11EffectPass* effectPass3 = IFcTechnique->GetPassByIndex( 0 );
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
	d3dDevice->CreateInputLayout( NoInstanceLayout, NoInstElements,
		shaderDesc.pBytecode, shaderDesc.BytecodeLength, &nonInstancedLayout_);
	d3dDevice->CreateInputLayout( NoInstanceNormalsLayout, NoInstNrmElements,
		shaderDesc2.pBytecode, shaderDesc2.BytecodeLength, &nonInstancedNormLayout_);
	d3dDevice->CreateInputLayout( InstanceLayout, InstElements,
		shaderDesc3.pBytecode, shaderDesc3.BytecodeLength, &instancedLayout_);
	buffer->Release( );
	*/
}

void ColourShader::preRender(ID3D11DeviceContext* d3dContext, void* data)
{	
	ShaderDataFC* d = (ShaderDataFC*)data;
	if (d->inputType == 0 || d->inputType == 3)
		d3dContext->IASetInputLayout( nonInstancedLayout_ );
	else if (d->inputType == 1)
		d3dContext->IASetInputLayout( nonInstancedNormLayout_ );
	else if (d->inputType == 4)
		d3dContext->IASetInputLayout( instancedLayout_ );
	ConstantBufferType* dataPtr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (ConstantBufferType*)mappedResource.pData;
	dataPtr->world		   = *d->worldMat;
	dataPtr->view		   = *d->viewMat;
	dataPtr->projection	   = *d->projMat;
	dataPtr->lightColour.x = d->lightCol[0];
	dataPtr->lightColour.y = d->lightCol[1];
	dataPtr->lightColour.z = d->lightCol[2];
	dataPtr->lightColour.w = d->lightCol[3];
	dataPtr->lightPos.x    = d->lightPos[0];
	dataPtr->lightPos.y    = d->lightPos[1];
	dataPtr->lightPos.z    = d->lightPos[2]; 
	dataPtr->camPos.x      = d->camPos[0];
	dataPtr->camPos.y      = d->camPos[1];
	dataPtr->camPos.z      = d->camPos[2];
    d3dContext->Unmap(constantBuffer, 0);
    d3dContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    d3dContext->VSSetShader(vertexShader, NULL, 0);
    d3dContext->PSSetShader(pixelShader, NULL, 0);
	//d3dContext->Draw( );

	/*
	ID3DX11EffectMatrixVariable* worldMatrix;
    worldMatrix = effect->GetVariableByName( "worldMatrix" )->AsMatrix( );
	worldMatrix->SetMatrix( ( float* )d->worldMat);
	ID3DX11EffectVectorVariable* camVec;
	camVec = effect->GetVariableByName( "camPos" )->AsVector( );
	camVec->SetFloatVector( d->camPos );
	ID3DX11EffectVectorVariable* lightVec;
	lightVec = effect->GetVariableByName( "lightPos" )->AsVector( );
	lightVec->SetFloatVector( d->lightPos );
	ID3DX11EffectVectorVariable* lightCVec;
	lightCVec = effect->GetVariableByName( "lightColourShader" )->AsVector( );
	lightCVec->SetFloatVector( d->lightCol);
    ID3DX11EffectMatrixVariable* viewMatrix;
    viewMatrix = effect->GetVariableByName( "viewMatrix" )->AsMatrix( );
    viewMatrix->SetMatrix( ( float* )d->viewMat );
    ID3DX11EffectMatrixVariable* projMatrix;
    projMatrix = effect->GetVariableByName( "projMatrix" )->AsMatrix( );
    projMatrix->SetMatrix( ( float* )d->projMat );
	*/
}