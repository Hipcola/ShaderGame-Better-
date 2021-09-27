#include"LSystemGPU.h"
#include"utility.h"
#include<time.h>

//I tell ya, I had a rash on my ball sack the other week. 
//It was so bad, Dustin Hoffman was travelling around with a photo off em trying to get me in quarantine.

LSystemGPU::LSystemGPU()
{
	inputSize = 0;
	inputParser = 0;
	testing123 = 0;
}

LSystemGPU::~LSystemGPU( )
{

}

struct sizeBuff
{
	unsigned int ruleSize;
	unsigned int tableSize;
	unsigned int inputSize;
	float timeIncre;
};

struct parserBuff
{
	unsigned int in;
	unsigned int b;
	unsigned int c;
	unsigned int d;
};

void LSystemGPU::Load(ID3D11Device* d3dDevice)
{
	//*
	ID3DBlob* buffer = 0;
	HRESULT h;
	bool compileResult;
	h = D3DReadFileToBlob(L"LSysCompute.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader);
	buffer->Release( );
	h = D3DReadFileToBlob(L"LSysCompute2.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader2);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserCompute.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseComp1);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserCompute2.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseComp2);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserCompute3.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseComp3);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserCompute4.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseComp4);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserCompute5.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseComp5);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserCompute6.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseComp6);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserComputeBD.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseCompBD);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserComputeBDG.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseCompBDG);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserComputeSU.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseCompSU);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserComputeNBD.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseCompNBD);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserComputeS1.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseCompS1);
	buffer->Release( );
	h = D3DReadFileToBlob(L"ParserComputeS2.cso", &buffer);
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &ParseCompS2);
	buffer->Release( );

	//h = D3DReadFileToBlob(L"VertConstruct.cso", &buffer);
	//compileResult = CompileD3DComputeShader(d3dDevice, buffer, &VertConstruct);
	//buffer->Release( );
	// Create the constant buffer pointer so we can access the compute shader constant buffer from within this class.
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (sizeof(unsigned int)*3) + sizeof(float);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	h = d3dDevice->CreateBuffer(&bufferDesc, NULL, &inputSize);
	//*/

	D3D11_BUFFER_DESC bufferDesc2;
    bufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc2.ByteWidth = sizeof(unsigned int)*4;
    bufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc2.MiscFlags = 0;
	bufferDesc2.StructureByteStride = 0;
	h = d3dDevice->CreateBuffer(&bufferDesc2, NULL, &inputParser);


	/*
	ID3DBlob* buffer = 0;
	char* CSTarget = (d3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)? "cs_5_0" : "cs_4_0";
	bool compileResult = CompileD3DShader( L"LSysCompute.hlsl", "CSMain", CSTarget, &buffer );
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader);
	buffer->Release( );
	compileResult = CompileD3DShader( L"LSysCompute2.hlsl", "CSMain", CSTarget, &buffer );
	compileResult = CompileD3DComputeShader(d3dDevice, buffer, &CShader2);
	buffer->Release( );
	// Create the constant buffer pointer so we can access the compute shader constant buffer from within this class.
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(unsigned int)*4;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	HRESULT test = d3dDevice->CreateBuffer(&bufferDesc, NULL, &inputSize);
	//*/
}

void LSystemGPU::run(ID3D11DeviceContext* d3dContext,
							   ID3D11ShaderResourceView** inSRV, 
							   ID3D11UnorderedAccessView** resultVersUAV, //ID3D11UnorderedAccessView*
							   UINT inSize, UINT ruleSize, UINT tableSize, float timeIncre)
{
	//Set data
	//ID3D11ShaderResourceView* aRViews[1] = { inSRV };
	//ID3D11UnorderedAccessView* uAViews[1] = { resultVersUAV }; 

	clock_t t;
	float duration;

	t = clock();
	d3dContext->CSSetShader( CShader, NULL, 0 );
	d3dContext->CSSetShaderResources( 0, 2, inSRV);
	d3dContext->CSSetUnorderedAccessViews( 0, 8, resultVersUAV/*uAViews*/, (UINT*)(&resultVersUAV) );
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dContext->Map(inputSize, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	sizeBuff* dataPtr = (sizeBuff*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->inputSize = inSize;
	dataPtr->tableSize = tableSize;
	dataPtr->ruleSize  = ruleSize;
	dataPtr->timeIncre = timeIncre;
	// Unlock the constant buffer.
    d3dContext->Unmap(inputSize, 0);
    d3dContext->CSSetConstantBuffers(0, 1, &inputSize);
	unsigned int dispatch = inSize/BLOCKSIZE;
	dispatch++;

	/*duration = (clock() - t);
	std::string a = "shader buffer setup = ";
	a.append(Utility::convertFloat(duration));
	a.append("\n");
	OutputDebugString(a.c_str());*/

	t = clock();
	d3dContext->Dispatch( dispatch, 1, 1 );
	if (dispatch > 1)
	{
		//Start second pass
		d3dContext->CSSetShader( CShader2, NULL, 0 );
		d3dContext->Dispatch( dispatch, 1, 1 );
	}

	ID3D11Buffer* ppCBNULL[1] = { NULL };
    d3dContext->CSSetConstantBuffers( 0, 1, ppCBNULL );
	ID3D11UnorderedAccessView* ppUAViewNULL[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    d3dContext->CSSetUnorderedAccessViews( 0, 8, ppUAViewNULL, NULL );
    ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
    d3dContext->CSSetShaderResources( 0, 2, ppSRVNULL );
	d3dContext->CSSetShader( NULL, NULL, 0 );
	/*
	duration = (clock() - t);
	a = "compshader run = ";
	a.append(Utility::convertFloat(duration));
	a.append("\n");
	OutputDebugString(a.c_str());
	*/
}

void LSystemGPU::parse(ID3D11DeviceContext* d3dContext, ID3D11Device* d3dDevice, 
							   ID3D11ShaderResourceView** inSRV, 
							   ID3D11Buffer** vers, ID3D11Buffer** inds, ID3D11ShaderResourceView** vertsSRV, ID3D11ShaderResourceView** indsSRV, ID3D11UnorderedAccessView** params,
							   UINT inSize, UINT& versOutput)
{
	d3dContext->CSSetShaderResources( 0, 3, inSRV);
	/*
	//int fdaf = 7%8;
	XMMATRIX m1 = XMMatrixSet
	(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 1, 0, 1
	);
	XMMATRIX m2 = XMMatrixSet
	(
		0, -1, 0, 0,
		1, 0, 0, 0,
		0, 0, 1, 0,
		4, 0, 0, 1
	);
	XMMATRIX m3 = XMMatrixMultiply(m1,m2);
	*/
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dContext->Map(inputParser, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	parserBuff* dataPtr = (parserBuff*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->in = inSize;
	dataPtr->b	= 0;
	dataPtr->c	= 0;
	dataPtr->d	= 0;
	// Unlock the constant buffer.
    d3dContext->Unmap(inputParser, 0);
	d3dContext->CSSetConstantBuffers(0, 1, &inputParser);

	unsigned int numPBlocks = (inSize/PARSECHUNK)+1;
	unsigned int dispatch = numPBlocks/BLOCKSIZE;
	dispatch++;
	ID3D11Buffer*               counters = NULL;
	ID3D11UnorderedAccessView*  countersUAV = NULL;
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(int)*4, numPBlocks, NULL, &counters);
	Utility::CreateBufferUAV(d3dDevice, counters, &countersUAV );
	ID3D11Buffer*               sumsP = NULL;
	ID3D11UnorderedAccessView*  sumsPUAV = NULL;
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*7, /*64*/dispatch+1, NULL, &sumsP);
	Utility::CreateBufferUAV(d3dDevice, sumsP, &sumsPUAV );

	ID3D11UnorderedAccessView* arry[3] = { *params, sumsPUAV, countersUAV };
	d3dContext->CSSetUnorderedAccessViews( 0, 3, arry, (UINT*)(&arry) );

	//unsigned int cSize = (inSize/PARSECHUNK)+1;
	d3dContext->CSSetShader( ParseComp1, NULL, 0 );
	d3dContext->Dispatch(dispatch,1,1);

	/*
	ID3D11Buffer* debugbuf6 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, counters );
	D3D11_MAPPED_SUBRESOURCE MappedResource6;
    d3dContext->Map( debugbuf6, 0, D3D11_MAP_READ, 0, &MappedResource6 );
	fourint* erter = (fourint*)MappedResource6.pData;	
	d3dContext->Unmap( debugbuf6, 0 );
	//*/

	d3dContext->CSSetShader( ParseComp2, NULL, 0 );
	d3dContext->Dispatch(1,1,1);

	ID3D11Buffer* debugbuf4 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsP );
	D3D11_MAPPED_SUBRESOURCE MappedResource4;
    d3dContext->Map( debugbuf4, 0, D3D11_MAP_READ, 0, &MappedResource4 );
	sevenUint* sumtest = (sevenUint*)MappedResource4.pData;	
	d3dContext->Unmap( debugbuf4, 0 );
	
	unsigned int versTotal	= sumtest[dispatch].a;
	unsigned int depthSize  = sumtest[dispatch].b;
	unsigned int bracketAdd	= sumtest[dispatch].c;
	versOutput = versTotal+1;

	/*
	if(dispatch == 2)
	{
		ID3D11Buffer* debugbuf444 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, counters );
		D3D11_MAPPED_SUBRESOURCE MappedResource444;
		d3dContext->Map( debugbuf444, 0, D3D11_MAP_READ, 0, &MappedResource444 );
		fourint* d = (fourint*)MappedResource444.pData;
		d3dContext->Unmap( debugbuf444, 0 );
	}
	//*/

	//If there are no brackets, preform a simpler parse system.
	if (depthSize == 0)
	{
		#pragma region No Brackets Pass
		//Output data Buffers
		ID3D11Buffer*               lOrien = NULL;
		ID3D11UnorderedAccessView*  lOrienUAV = NULL;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(XMFLOAT4X4), dispatch, NULL, &lOrien );
		Utility::CreateBufferUAV( d3dDevice, lOrien, &lOrienUAV );
		ID3D11UnorderedAccessView*  vertsUAV = NULL;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(float)*4, versOutput, NULL, vers );
		Utility::CreateBufferSRV( d3dDevice, *vers, vertsSRV );
		Utility::CreateBufferUAV( d3dDevice, *vers, &vertsUAV );
		ID3D11UnorderedAccessView*  indsUAV = NULL;
		unsigned int indsCheck = (versOutput-1)*2;
		if (versOutput == 1)
			indsCheck = 2;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(unsigned int), indsCheck, NULL, inds );
		Utility::CreateBufferSRV( d3dDevice, *inds, indsSRV );
		Utility::CreateBufferUAV( d3dDevice, *inds, &indsUAV );
		ID3D11UnorderedAccessView* ary[3] = { lOrienUAV, vertsUAV, indsUAV };
		d3dContext->CSSetUnorderedAccessViews( 3, 3, ary, (UINT*)(&ary) );

		d3dContext->CSSetShader( ParseCompS1, NULL, 0 );
		d3dContext->Dispatch( dispatch,1,1 );

		//NEED ANOTHER SUM DISPATCH HERE

		d3dContext->CSSetShader( ParseCompS2, NULL, 0 );
		d3dContext->Dispatch( dispatch,1,1 );

		/*
		ID3D11Buffer* debugbuf45 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, *vers );
		D3D11_MAPPED_SUBRESOURCE MappedResource45;
		d3dContext->Map( debugbuf45, 0, D3D11_MAP_READ, 0, &MappedResource45 );
		XMFLOAT4* postest = (XMFLOAT4*)MappedResource45.pData;
		d3dContext->Unmap( debugbuf45, 0 );
	
		ID3D11Buffer* debugbuf77 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, *inds );
		D3D11_MAPPED_SUBRESOURCE MappedResource77;
		d3dContext->Map( debugbuf77, 0, D3D11_MAP_READ, 0, &MappedResource77 );
		unsigned int* counts2 = (unsigned int*)MappedResource77.pData;	
		d3dContext->Unmap( debugbuf77, 0 );

		ID3D11Buffer* debugbuf80 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, lOrien );
		D3D11_MAPPED_SUBRESOURCE MappedResource80;
		d3dContext->Map( debugbuf80, 0, D3D11_MAP_READ, 0, &MappedResource80 );
		XMFLOAT4X4* rotstest = (XMFLOAT4X4*)MappedResource80.pData;	
		d3dContext->Unmap( debugbuf80, 0 );
		//*/

		ID3D11UnorderedAccessView* aryUAVNULL[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
		d3dContext->CSSetUnorderedAccessViews( 0, 6, aryUAVNULL, (UINT*)(&aryUAVNULL) );

		ID3D11ShaderResourceView* arySRVNULL[3] = { NULL, NULL, NULL };
		d3dContext->CSSetShaderResources( 0, 3, arySRVNULL);

		#pragma endregion 
	}
	else
	{
		/*
		if (depthSize > 1)
		{
			int afasfd = 123432;
		}
		//*/
		ID3D11Buffer* depthSizes;
		ID3D11UnorderedAccessView* depthSizesUAV;
		ID3D11Buffer* brackets;
		ID3D11UnorderedAccessView* bracketsUAV;
		ID3D11Buffer* depthArrys;
		ID3D11UnorderedAccessView* depthArrysUAV;
		Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), (BLOCKSIZE*depthSize) * dispatch, NULL, &depthArrys);
		Utility::CreateBufferUAV(d3dDevice, depthArrys, &depthArrysUAV);
		Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*7, bracketAdd, NULL, &brackets);
		Utility::CreateBufferUAV(d3dDevice, brackets, &bracketsUAV);
		Utility::CreateDrawIndirectBuffer(d3dDevice, sizeof(unsigned int)*3, /*depthSize+1*/depthSize+2, NULL, &depthSizes);
		Utility::CreateBufferUAV(d3dDevice, depthSizes, &depthSizesUAV);
		ID3D11UnorderedAccessView* ary2[3] = {depthSizesUAV, bracketsUAV, depthArrysUAV };
		d3dContext->CSSetUnorderedAccessViews( 5, 3, ary2, (UINT*)(&ary2) );

		d3dContext->Map(inputParser, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		// Get a pointer to the data in the constant buffer.
		dataPtr = (parserBuff*)mappedResource.pData;
		// Copy the matrices into the constant buffer.
		dataPtr->in = inSize;
		dataPtr->b	= depthSize;
		dataPtr->c	= bracketAdd;
		dataPtr->d	= 0;
		// Unlock the constant buffer.
		d3dContext->Unmap(inputParser, 0);

		//clock_t t;
		//t = clock();

		ID3D11Buffer*               sumsN = NULL;
		ID3D11UnorderedAccessView*  sumsNUAV = NULL;
		Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), depthSize * dispatch, NULL, &sumsN);
		Utility::CreateBufferUAV(d3dDevice, sumsN, &sumsNUAV );
		d3dContext->CSSetUnorderedAccessViews( 3, 1, &sumsNUAV, (UINT*)(&sumsNUAV) );

		d3dContext->CSSetShader( ParseComp3, NULL, 0 );
		d3dContext->Dispatch(dispatch,1,1);

		/*
		if(dispatch == 2)
		{
			ID3D11Buffer* debugbuf444 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, counters );
			D3D11_MAPPED_SUBRESOURCE MappedResource444;
			d3dContext->Map( debugbuf444, 0, D3D11_MAP_READ, 0, &MappedResource444 );
			fourint* d = (fourint*)MappedResource444.pData;
			d3dContext->Unmap( debugbuf444, 0 );

			ID3D11Buffer* debugbuf4 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsN );
			D3D11_MAPPED_SUBRESOURCE MappedResource4;
			d3dContext->Map( debugbuf4, 0, D3D11_MAP_READ, 0, &MappedResource4 );
			unsigned int* depthtestF = (unsigned int*)MappedResource4.pData;
			d3dContext->Unmap( debugbuf4, 0 );
		}
		//*/

		/*
		if(depthSize >= 11)
		{
			ID3D11Buffer* debugbuf4 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsN );
			D3D11_MAPPED_SUBRESOURCE MappedResource4;
			d3dContext->Map( debugbuf4, 0, D3D11_MAP_READ, 0, &MappedResource4 );
			unsigned int* depthtestF = (unsigned int*)MappedResource4.pData;
			d3dContext->Unmap( debugbuf4, 0 );
			int sdaf = 2342;
		}
		//*/

		d3dContext->CSSetShader( ParseCompBDG, NULL, 0 );
		d3dContext->Dispatch(1,1,1);

		/*
		if(depthSize >= 21)
		{
			ID3D11Buffer* debugbuf4 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsN );
			D3D11_MAPPED_SUBRESOURCE MappedResource4;
			d3dContext->Map( debugbuf4, 0, D3D11_MAP_READ, 0, &MappedResource4 );
			unsigned int* depthtestF = (unsigned int*)MappedResource4.pData;
			d3dContext->Unmap( debugbuf4, 0 );
			
			ID3D11Buffer* debugbuf23 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthSizes );
			D3D11_MAPPED_SUBRESOURCE MappedResource23;
			d3dContext->Map( debugbuf23, 0, D3D11_MAP_READ, 0, &MappedResource23 );
			unsigned int* dSize = (unsigned int*)MappedResource23.pData;
			d3dContext->Unmap( debugbuf23, 0 );
		}
		//*/

		/*
		//dispatch > 1
		if(dispatch == 2)
		{
			ID3D11Buffer* debugbuf27 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthArrys );
			D3D11_MAPPED_SUBRESOURCE MappedResource27;
			d3dContext->Map( debugbuf27, 0, D3D11_MAP_READ, 0, &MappedResource27 );
			unsigned int* counts22 = (unsigned int*)MappedResource27.pData;
			d3dContext->Unmap( debugbuf27, 0 );	

			ID3D11Buffer* debugbuf444 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsN );
			D3D11_MAPPED_SUBRESOURCE MappedResource444;
			d3dContext->Map( debugbuf444, 0, D3D11_MAP_READ, 0, &MappedResource444 );
			unsigned int* depthtestF = (unsigned int*)MappedResource444.pData;
			d3dContext->Unmap( debugbuf444, 0 );

			ID3D11Buffer* debugbuf23 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthSizes );
			D3D11_MAPPED_SUBRESOURCE MappedResource23;
			d3dContext->Map( debugbuf23, 0, D3D11_MAP_READ, 0, &MappedResource23 );
			unsigned int* dSize = (unsigned int*)MappedResource23.pData;
			d3dContext->Unmap( debugbuf23, 0 );
		}
		//*/

		d3dContext->CSSetShader( ParseComp4, NULL, 0 );
		d3dContext->Dispatch(dispatch,1,1);

		/*
		clock_t t2;
		t2 = clock();
		float duration = (clock() - t);
		*/
		ID3D11UnorderedAccessView* UAVNULL = NULL;
		d3dContext->CSSetUnorderedAccessViews( 1, 1, &UAVNULL, NULL );
		d3dContext->CSSetUnorderedAccessViews( 3, 1, &UAVNULL, NULL );
		d3dContext->CSSetUnorderedAccessViews( 7, 1, &UAVNULL, NULL );
		Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*7, (bracketAdd/2)+1, NULL, &sumsP);
		Utility::CreateBufferUAV(d3dDevice, sumsP, &sumsPUAV );
		d3dContext->CSSetUnorderedAccessViews( 1, 1, &sumsPUAV, (UINT*)(&sumsPUAV) );
		
		d3dContext->CSSetShader( ParseCompNBD, NULL, 0 );
		d3dContext->Dispatch( (bracketAdd/2)+1,1,1 );

		/*
		if(dispatch == 2)
		{	
			ID3D11Buffer* debugbuf444 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsP );
			D3D11_MAPPED_SUBRESOURCE MappedResource444;
			d3dContext->Map( debugbuf444, 0, D3D11_MAP_READ, 0, &MappedResource444 );
			sevenUint* depthtestF = (sevenUint*)MappedResource444.pData;
			d3dContext->Unmap( debugbuf444, 0 );

			ID3D11Buffer* debugbuf12 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, brackets );
			D3D11_MAPPED_SUBRESOURCE MappedResource12;
			d3dContext->Map( debugbuf12, 0, D3D11_MAP_READ, 0, &MappedResource12 );
			sevenUint* partest = (sevenUint*)MappedResource12.pData;	
			d3dContext->Unmap( debugbuf12, 0 );

			ID3D11Buffer* debugbuf27 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthSizes );
			D3D11_MAPPED_SUBRESOURCE MappedResource27;
			d3dContext->Map( debugbuf27, 0, D3D11_MAP_READ, 0, &MappedResource27 );
			unsigned int* counts22 = (unsigned int*)MappedResource27.pData;
			d3dContext->Unmap( debugbuf27, 0 );
		}
		//*/

		ID3D11Buffer*               test = NULL;
		ID3D11UnorderedAccessView*  testUAV = NULL;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(unsigned int)*2, 1, NULL, &test );
		Utility::CreateBufferUAV( d3dDevice, test, &testUAV );
		d3dContext->CSSetUnorderedAccessViews( 7, 1, &testUAV, (UINT*)(&testUAV) );

		d3dContext->CSSetShader( ParseCompSU, NULL, 0 );
		d3dContext->Dispatch( 1,1,1 );

		/*
		if (dispatch == 2)
		{
			ID3D11Buffer* debugbuf27 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthSizes );
			D3D11_MAPPED_SUBRESOURCE MappedResource27;
			d3dContext->Map( debugbuf27, 0, D3D11_MAP_READ, 0, &MappedResource27 );
			unsigned int* counts22 = (unsigned int*)MappedResource27.pData;
			d3dContext->Unmap( debugbuf27, 0 );
		}
		//*/

		ID3D11Buffer* calcResult = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, test );
		D3D11_MAPPED_SUBRESOURCE cRMap;
		d3dContext->Map( calcResult, 0, D3D11_MAP_READ, 0, &cRMap );
		twoUint* results = (twoUint*)cRMap.pData;
		d3dContext->Unmap( calcResult, 0 );
	
		d3dContext->CSSetUnorderedAccessViews( 7, 1, &UAVNULL, NULL );						// Not the correct size
		Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*7, (bracketAdd/2)+1/*results->a*/ , NULL, &sumsN);
		Utility::CreateBufferUAV(d3dDevice, sumsN, &sumsNUAV );
		d3dContext->CSSetUnorderedAccessViews( 7, 1, &sumsNUAV, (UINT*)(&sumsNUAV) );

		d3dContext->CSSetUnorderedAccessViews( 2, 1, &UAVNULL, NULL );
		counters->Release();
		countersUAV->Release();
		counters = NULL;
		countersUAV = NULL;
		Utility::CreateStructuredBuffer(d3dDevice, sizeof(int)*4, results->a, NULL, &counters);
		Utility::CreateBufferUAV(d3dDevice, counters, &countersUAV );
		d3dContext->CSSetUnorderedAccessViews( 2, 1, &countersUAV, (UINT*)(&countersUAV) );

		d3dContext->CSSetShader( ParseCompBD, NULL, 0 );
		d3dContext->Dispatch( (bracketAdd/2)+1,1,1 );

		/*
		if (dispatch == 2)
		{
			ID3D11Buffer* debugbuf99 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, counters );
			D3D11_MAPPED_SUBRESOURCE MappedResource99;
			d3dContext->Map( debugbuf99, 0, D3D11_MAP_READ, 0, &MappedResource99 );
			fourint* paratest2 = (fourint*)MappedResource99.pData;
			d3dContext->Unmap( debugbuf99, 0 );

			ID3D11Buffer* debugbuf46 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, sumsN );
			D3D11_MAPPED_SUBRESOURCE MappedResource46;
			d3dContext->Map( debugbuf46, 0, D3D11_MAP_READ, 0, &MappedResource46 );
			sevenUint* depthtest2 = (sevenUint*)MappedResource46.pData;
			d3dContext->Unmap( debugbuf46, 0 );

			ID3D11Buffer* debugbuf12 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, brackets );
			D3D11_MAPPED_SUBRESOURCE MappedResource12;
			d3dContext->Map( debugbuf12, 0, D3D11_MAP_READ, 0, &MappedResource12 );
			sevenUint* partest = (sevenUint*)MappedResource12.pData;	
			d3dContext->Unmap( debugbuf12, 0 );

			ID3D11Buffer* debugbuf27 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthSizes );
			D3D11_MAPPED_SUBRESOURCE MappedResource27;
			d3dContext->Map( debugbuf27, 0, D3D11_MAP_READ, 0, &MappedResource27 );
			unsigned int* counts22 = (unsigned int*)MappedResource27.pData;
			d3dContext->Unmap( debugbuf27, 0 );
		}
		//*/

		/*
		if (results->a > 999999)
		{
		}*/

		ID3D11UnorderedAccessView*  vertsUAV = NULL;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(float)*4, versOutput, NULL, vers );
		Utility::CreateBufferSRV( d3dDevice, *vers, vertsSRV );
		Utility::CreateBufferUAV( d3dDevice, *vers, &vertsUAV );
		ID3D11UnorderedAccessView*  indsUAV = NULL;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(unsigned int), (versOutput-1)*2, NULL, inds );
		Utility::CreateBufferSRV( d3dDevice, *inds, indsSRV );
		Utility::CreateBufferUAV( d3dDevice, *inds, &indsUAV );
		ID3D11UnorderedAccessView* ary[2] = { vertsUAV, indsUAV };
		d3dContext->CSSetUnorderedAccessViews( 3, 2, ary, (UINT*)(&ary) );

		d3dContext->CSSetUnorderedAccessViews( 1, 1, &UAVNULL, NULL );
		ID3D11Buffer*               lOrien = NULL;
		ID3D11UnorderedAccessView*  lOrienUAV = NULL;
		Utility::CreateStructuredBuffer( d3dDevice, sizeof(XMFLOAT4X4), (results->a)+(bracketAdd/2) + 32, NULL, &lOrien );
		Utility::CreateBufferUAV( d3dDevice, lOrien, &lOrienUAV );
		d3dContext->CSSetUnorderedAccessViews( 1, 1, &lOrienUAV, (UINT*)(&lOrienUAV) );

		///*
		for (unsigned int i = 0; i<=depthSize; i++)
		{
			d3dContext->Map(inputParser, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			// Get a pointer to the data in the constant buffer.
			dataPtr = (parserBuff*)mappedResource.pData;
			// Copy the matrices into the constant buffer.
			dataPtr->in = inSize;
			dataPtr->b	= depthSize;
			dataPtr->c	= bracketAdd;
			dataPtr->d	= i;
			// Unlock the constant buffer.
			d3dContext->Unmap( inputParser, 0 );

			d3dContext->CSSetShader( ParseComp5, NULL, 0 );
			//d3dContext->Dispatch( 1, 1 , 1 );
			d3dContext->DispatchIndirect( depthSizes, 0 );

			/*
			if (depthSize == 4)
			{
				ID3D11Buffer* debugbuf84 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, lOrien );
				D3D11_MAPPED_SUBRESOURCE MappedResource84;
				d3dContext->Map( debugbuf84, 0, D3D11_MAP_READ, 0, &MappedResource84 );
				XMFLOAT4X4* rotstestF = (XMFLOAT4X4*)MappedResource84.pData;	
				d3dContext->Unmap( debugbuf84, 0 );
			}
			//*/

			d3dContext->CSSetShader( ParseComp6, NULL, 0 );
			d3dContext->DispatchIndirect( depthSizes, 0 );

			/*
			if (depthSize > 1)
			{
				ID3D11Buffer* debugbuf88 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, lOrien );
				D3D11_MAPPED_SUBRESOURCE MappedResource88;
				d3dContext->Map( debugbuf88, 0, D3D11_MAP_READ, 0, &MappedResource88 );
				XMFLOAT4X4* rotstest = (XMFLOAT4X4*)MappedResource88.pData;	
				d3dContext->Unmap( debugbuf88, 0 );

				ID3D11Buffer* debugbuf45 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, *vers );
				D3D11_MAPPED_SUBRESOURCE MappedResource45;
				d3dContext->Map( debugbuf45, 0, D3D11_MAP_READ, 0, &MappedResource45 );
				XMFLOAT4* postest = (XMFLOAT4*)MappedResource45.pData;	
				d3dContext->Unmap( debugbuf45, 0 );

				ID3D11Buffer* debugbuf77 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, *inds );
				D3D11_MAPPED_SUBRESOURCE MappedResource77;
				d3dContext->Map( debugbuf77, 0, D3D11_MAP_READ, 0, &MappedResource77 );
				unsigned int* counts2 = (unsigned int*)MappedResource77.pData;	
				d3dContext->Unmap( debugbuf77, 0 );
				
				ID3D11Buffer* debugbuf27 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, depthSizes );
				D3D11_MAPPED_SUBRESOURCE MappedResource27;
				d3dContext->Map( debugbuf27, 0, D3D11_MAP_READ, 0, &MappedResource27 );
				unsigned int* counts22 = (unsigned int*)MappedResource27.pData;
				d3dContext->Unmap( debugbuf27, 0 );
			}
			//*/
		}
		ID3D11UnorderedAccessView* aryUAVNULL[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		d3dContext->CSSetUnorderedAccessViews( 0, 8, aryUAVNULL, (UINT*)(&aryUAVNULL) );

		ID3D11ShaderResourceView* arySRVNULL[3] = { NULL, NULL, NULL };
		d3dContext->CSSetShaderResources( 0, 3, arySRVNULL);

		SAFE_RELEASE(calcResult);
	}
	SAFE_RELEASE(debugbuf4);

	store = depthSize;
}