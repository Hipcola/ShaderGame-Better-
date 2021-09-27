#include"Text.h"
#include"DDSTextureLoader.h"
#include"GlobalData.h"

Text::Text() : shaderTexture_( 0 ), colorMapSampler_( 0 )
{

}

Text::~Text( )
{
	if( colorMapSampler_ ) colorMapSampler_->Release( );
    if( shaderTexture_ ) shaderTexture_->Release( );
    if( effect ) effect->Release( );
	if( layout_ ) layout_->Release( );
    colorMapSampler_ = 0;
    shaderTexture_ = 0;
    effect = 0;
    layout_ = 0;
}

void Text::fileNameLoad(ID3D11Device* d3dDevice, char* fName)
{
	texFileName = fName;
	Load(d3dDevice);
}

void Text::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	//MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", "empty", MB_OK);

	return;
}

void Text::Load(ID3D11Device* d3dDevice)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	//ID3D10Blob* vertexShaderBuffer;
	//ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC constantBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC pixelBufferDesc;

	ID3DBlob* buffer;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	//vertexShaderBuffer = 0;
	//pixelShaderBuffer = 0;

    // Compile the pixel shader code.
	D3DReadFileToBlob(L"fontP.cso", &buffer);
	//result = D3DX11CompileFromFile("font.ps", NULL, NULL, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
	//							   &pixelShaderBuffer, &errorMessage, NULL);

    // Create the vertex shader from the buffer.
    d3dDevice->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), NULL, 
									   &m_pixelShader);
	buffer->Release( );

	// Compile the vertex shader code.
	D3DReadFileToBlob(L"fontV.cso", &buffer);
	    // Create the vertex shader from the buffer.
    d3dDevice->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), NULL, 
										&m_vertexShader);

	//result = D3DX11CompileFromFile("font.vs", NULL, NULL, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
	//							   &vertexShaderBuffer, &errorMessage, NULL);



	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	d3dDevice->CreateInputLayout(polygonLayout, numElements, buffer->GetBufferPointer(), 
									   buffer->GetBufferSize(), &m_layout);
	if(FAILED(result))
	{
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	//vertexShaderBuffer->Release();
	//vertexShaderBuffer = 0;

	//pixelShaderBuffer->Release();
	//pixelShaderBuffer = 0;

    // Setup the description of the dynamic constant buffer that is in the vertex shader.
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	d3dDevice->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);
	if(FAILED(result))
	{
	}

	// Create a texture sampler state description.
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
    d3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
	}

    // Setup the description of the dynamic pixel constant buffer that is in the pixel shader.
    pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
    pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pixelBufferDesc.MiscFlags = 0;
	pixelBufferDesc.StructureByteStride = 0;

	// Create the pixel constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	d3dDevice->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);
	if(FAILED(result))
	{
	}

	//Texture
	result = CreateDDSTextureFromFile(d3dDevice, L"fontN.dds", nullptr, &texture);
	//result = D3DX11CreateShaderResourceViewFromFile(d3dDevice, "font.dds" , NULL, NULL, &texture, NULL);
}

void Text::preRender(ID3D11DeviceContext* d3dContext, void* data)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBufferType* dataPtr;
	unsigned int bufferNumber;
	PixelBufferType* dataPtr2;


	// Lock the constant buffer so it can be written to.
	result = d3dContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (ConstantBufferType*)mappedResource.pData;

	XMMATRIX view;
	XMVECTOR up;
	XMVECTOR position;
	XMVECTOR lookAt;
	up = XMVectorSet(0,1.f,0,0);
	position = XMVectorSet(0,0,-1.f,0);
	lookAt = XMVectorSet(0,0,0,0);

	// Copy the matrices into the constant buffer.
	dataPtr->world = XMMatrixIdentity();
	dataPtr->view = XMMatrixLookAtLH(position, lookAt, up);
	dataPtr->projection = XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 1000.f);

	// Unlock the constant buffer.
    d3dContext->Unmap(m_constantBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
    d3dContext->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

	// Set shader texture resource in the pixel shader.
	d3dContext->PSSetShaderResources(0, 1, &texture);

	// Lock the pixel constant buffer so it can be written to.
	result = d3dContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
	}

	// Get a pointer to the data in the pixel constant buffer.
	dataPtr2 = (PixelBufferType*)mappedResource.pData;

	ShaderDataText* d = (ShaderDataText*)data;
	// Copy the pixel color into the pixel constant buffer.
	dataPtr2->pixelColor = *d->colour;

	// Unlock the pixel constant buffer.
    d3dContext->Unmap(m_pixelBuffer, 0);

	// Set the position of the pixel constant buffer in the pixel shader.
	bufferNumber = 0;

	// Now set the pixel constant buffer in the pixel shader with the updated value.
    d3dContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelBuffer);

	d3dContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render the triangles.
    d3dContext->VSSetShader(m_vertexShader, NULL, 0);
    d3dContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	d3dContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangles.
	d3dContext->DrawIndexed(*d->indCount, 0, 0);


}