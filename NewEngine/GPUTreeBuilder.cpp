#include"GPUTreeBuilder.h"
#include"utility.h"

GPUTreeBuilder::GPUTreeBuilder()
{
	texCoords = NULL;
	texCoordsSRV = NULL;
	inSides = 0;
}

GPUTreeBuilder::~GPUTreeBuilder( )
{
	
}

struct sizeBuff
{
	unsigned int vertSize;
	unsigned int indSize;
	unsigned int sides;
	unsigned int pad;
};

void GPUTreeBuilder::Load(ID3D11Device* d3dDevice, unsigned int sides)
{
	ID3DBlob* buffer = 0;

	HRESULT h;
	bool compileResult;
	h = D3DReadFileToBlob(L"TreeCompute2.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader);
	buffer->Release( );
	
	/*
	h = D3DReadFileToBlob(L"TreeCompute2.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader);
	buffer->Release( );
	*/

	/*char* CSTarget = (d3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)? "cs_5_0" : "cs_4_0";
	bool compileResult = CompileD3DShader( L"TreeCompute2.hlsl", "CSMain", CSTarget, &buffer );
    if( compileResult == false )
    {
    }
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader);
	buffer->Release( );*/

	//*
	// Create the constant buffer pointer so we can access the compute shader constant buffer from within this class.
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(unsigned int)*4;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	HRESULT test = d3dDevice->CreateBuffer(&bufferDesc, NULL, &inputSize);

	float* texcoord = new float[sides];
	float texCount = 1.0f/(sides-1);
	float count = 0;
	for (int a = 0;a<sides;a++){
		texcoord[a] = count;
		count += texCount;
	}
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), sides, texcoord, &texCoords);
	Utility::CreateBufferSRV(d3dDevice, texCoords, &texCoordsSRV );

	inSides = sides;
	delete texcoord;
	//*/

}

void GPUTreeBuilder::buildTree(ID3D11DeviceContext* d3dContext, 
							   ID3D11ShaderResourceView* versSRV, 
							   ID3D11ShaderResourceView* indsSRV, 
							   ID3D11UnorderedAccessView* resultVersUAV,
							   ID3D11UnorderedAccessView* resultIndsUAV,
							   UINT vertSize, UINT indSize, ID3D11Device* d3dDevice)
{
	///*
	//Set data
	ID3D11ShaderResourceView* aRViews[3]  = { versSRV, indsSRV, texCoordsSRV };
	ID3D11UnorderedAccessView* uAViews[2] = { resultVersUAV, resultIndsUAV }; 
	d3dContext->CSSetShader( CShader, NULL, 0 );
	d3dContext->CSSetShaderResources( 0, 3, aRViews );
	d3dContext->CSSetUnorderedAccessViews( 0, 2, uAViews, (UINT*)(&uAViews) );
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dContext->Map(inputSize, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	sizeBuff* dataPtr = (sizeBuff*)mappedResource.pData;
	dataPtr->vertSize = vertSize;
	dataPtr->indSize = indSize;
	dataPtr->sides = inSides;
	dataPtr->pad = 0;
    d3dContext->Unmap(inputSize, 0);

    d3dContext->CSSetConstantBuffers(0, 1, &inputSize);
	unsigned int dispatch = indSize/2;
	//dispatch++;
	d3dContext->Dispatch( dispatch, 1, 1 );

	//unset data
	ID3D11UnorderedAccessView* ppUAViewNULL[2] = { NULL, NULL };
    d3dContext->CSSetUnorderedAccessViews( 0, 2, ppUAViewNULL, (UINT*)(&uAViews) );
    ID3D11ShaderResourceView* ppSRVNULL[3] = { NULL, NULL, NULL };
    d3dContext->CSSetShaderResources( 0, 3, ppSRVNULL );

	/*
	ID3D11ShaderResourceView* aRViews[2] = { versSRV, indsSRV };
	d3dContext->CSSetShaderResources( 0, 2, aRViews);
	ID3D11UnorderedAccessView* uAViews[1] = { resultVersUAV}; 
	d3dContext->CSSetShader( CShader, NULL, 0 );;
	d3dContext->CSSetUnorderedAccessViews( 0, 1, uAViews, (UINT*)(&uAViews) );
	d3dContext->Dispatch( indSize, 1, 1 );
	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
    d3dContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, (UINT*)(&uAViews) );
	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL};
	d3dContext->CSSetShaderResources( 0, 2, ppSRVNULL );
	//*/
}
