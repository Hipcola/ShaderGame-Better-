#include"RawVerts.h"
#include"GlobalData.h"

RawVerts::RawVerts()
{
	XMStoreFloat4x4(&rotationMat,XMMATRIX());
	XMStoreFloat4x4(&worldMat,XMMATRIX());
	position = XMFLOAT3(0,0,0);

	isLineList = false;

	//added 6/11

	stride = 0;
	instStride = 0;

	numVBuffers = 1;
	numIBuffers = 1;
	numInstBuffers = 1;
	vertexCount = new unsigned int[numVBuffers];
	indicesCount = new unsigned int[numIBuffers];
	instanceCount = new unsigned int[numInstBuffers];
	vertexCount[0] = 0;
	indicesCount[0] = 0;
	instanceCount[0] = 0;
	vertexBuffers.resize(1);
	indexBuffers.resize(1);
	instanceBuffers.resize(1);
	vertexBuffers[0] = NULL;
	indexBuffers[0] = NULL;
	instanceBuffers[0] = NULL;

}

RawVerts::~RawVerts( )
{
	Clear();
}

void RawVerts::setFlatSurface(ID3D11Device* d3dDevice, XMFLOAT2 size, XMFLOAT2 texSpread, XMFLOAT3 direction, UINT columns, UINT rows)
{
	float rowPos = position.x-(size.x/2);
	float rowSpacing = (size.x/rows);
	float columnPos = position.z-(size.y/2);
	float columnSpacing = (size.y/columns);
	float xTexSpread = texSpread.x/(rows-1);
	float yTexSpread = texSpread.y/(columns-1);
	XMVECTOR defaultDirection = XMVectorSet(0,1,0,0);
	VertexPosRVTex* verts = new VertexPosRVTex[rows * columns];
	for(int row = 0; row<rows; row++)
	{
		for(int column = 0; column<columns; column++)
		{
			//const XMFLOAT3* d = &direction;
			/*XMVECTOR newDirection = XMLoadFloat3(&direction);
			/*float scalar = *XMVector3Dot(newDirection,defaultDirection).m128_f32;
			float v1 = *XMVector3Length(newDirection).m128_f32;
			float v2 = *XMVector3Length(defaultDirection).m128_f32;
			float theta = scalar/(v1*v2);
			XMVECTOR axis = XMVector3Cross(newDirection,defaultDirection);
			float check = *XMVector3Length(axis).m128_f32;*/
			//XMFLOAT3 v;
			/*if (check >0){
				XMMATRIX m = XMMatrixRotationAxis(axis,1.57079633);
				newDirection = XMVector3Transform(XMVectorSet(rowPos,0,columnPos,0),m);
				XMStoreFloat3(&v,newDirection);
			}
			else { */
			//v= XMFLOAT3(rowPos,0,columnPos); //}
			verts[row + column * rows].pos = XMFLOAT3(rowPos,0,columnPos);
			verts[row + column * rows].tex0 = XMFLOAT2(row*xTexSpread,column*yTexSpread);
			columnPos += columnSpacing;
		}
		columnPos = position.z-(size.y/2);
		rowPos += rowSpacing;
	}

	//WORD* inds = new WORD[(rows - 1) * (columns - 1) * 3];
	WORD* inds = new WORD[(rows - 1) * (columns - 1) * 6];
	UINT counter = 0;
	for(int row = 0; row<rows-1; row++){
		for(int column = 0; column<columns-1; column++){
			int lowerLeft = row + column*rows;
			int lowerRight = (row + 1) + column*rows;
			int topLeft = row + (column + 1) * rows;
			int topRight = (row + 1) + (column + 1) * rows;
 
			inds[counter++] = topLeft;
			inds[counter++] = lowerRight;
			inds[counter++] = lowerLeft;
			inds[counter++] = topLeft;
            inds[counter++] = topRight;
            inds[counter++] = lowerRight;

		}
	}

	Load(d3dDevice,
		verts, sizeof(VertexPosRVTex), rows * columns, 
		inds, sizeof(WORD), (rows - 1) * (columns - 1) * 6, 
		NULL, NULL, 0);
}

void RawVerts::setDome(ID3D11Device* d3dDevice, UINT detail, float radius)
{
	// Create and fill vertex buffer
	VertexPosRV* verts = new VertexPosRV[detail * detail + detail];
	// Create sphere vertices
	int counter = 0;
	//float radius = size;
	float pi = 3.141592654f;
	// from phi = -pi to pi
	for(float phi = -pi ; phi < pi ; phi += (2*pi/(float)detail) )
	{
		// from theta = 0 to 2pi
		for(float theta = 0 ; theta < 2*pi; theta += (2*pi/(float)detail) )
		{
			verts[counter].pos = XMFLOAT3(radius*cos(phi)*cos(theta),radius*sin(phi),radius*cos(phi)*sin(theta));
			counter++;
		}
	}

	// Create index buffer
    WORD* inds= new WORD[detail * detail * 6];

	int i = 0;
	int curPoint, offset;
	for(int deltaV=0; deltaV<detail; deltaV++)
	{
		for(int deltaU=0; deltaU<detail; deltaU++)
		{
			curPoint = deltaV*detail + deltaU;

			//Wrap edges
			if( (curPoint+1)%detail == 0 )
				offset = detail;
			else offset = 0;

			inds[i++] = curPoint; // 1
			inds[i++] = curPoint + 1 - offset; // 4
			inds[i++] = curPoint + 1 + detail - offset; // 3

			inds[i++] = curPoint; // 1
			inds[i++] = curPoint + 1 + detail - offset; // 3
			inds[i++] = curPoint + detail; // 2
		}
	}

	Load(d3dDevice,
		verts, sizeof(VertexPosRV), detail * detail, 
		inds, sizeof(WORD), detail * detail * 6, 
		NULL, NULL, 0);
}

void RawVerts::setPosition(XMFLOAT3 p)
{
	position = p;
	XMMATRIX translationMat = XMMatrixTranslation( position.x, position.y, position.z );
	XMStoreFloat4x4(&worldMat,(XMLoadFloat4x4(&rotationMat) * translationMat));
}

XMFLOAT3 RawVerts::getPosition()
{
	return position;
}

void RawVerts::setRotation(XMMATRIX* r)
{
	XMStoreFloat4x4(&rotationMat,*r);
	XMMATRIX translationMat = XMMatrixTranslation( position.x, position.y, position.z );
    XMStoreFloat4x4(&worldMat,(*r * translationMat));
}

void RawVerts::Load(ID3D11Device* d3dDevice,
											void* verts, unsigned int vertbyteSize, unsigned int numOfVerts,
											void* indices, unsigned int indbyteSize, unsigned int numOfInds,
											void* instances, unsigned int InstbyteSize, unsigned int numOfInsts)
{
	//Clear();
	D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory( &resourceData, sizeof( resourceData ) );
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	if (numOfVerts > 0)
	{
		vertexCount[0] = numOfVerts;
		D3D11_BUFFER_DESC vertexDesc;
		ZeroMemory( &vertexDesc, sizeof( vertexDesc ) );
		vertexDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.ByteWidth = vertbyteSize * numOfVerts;
		resourceData.pSysMem = verts;
		d3dDevice->CreateBuffer( &vertexDesc, &resourceData, &vertexBuffers[0] );

		stride = vertbyteSize;
	}

	if (numOfInds > 0)
	{
		indicesCount[0] = numOfInds;
		D3D11_BUFFER_DESC indexDesc;
		ZeroMemory( &indexDesc, sizeof( indexDesc ) );
		indexDesc.Usage = D3D11_USAGE_DEFAULT;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.ByteWidth = indbyteSize * numOfInds;
		indexDesc.CPUAccessFlags = 0;
		resourceData.pSysMem = indices;
		d3dDevice->CreateBuffer( &indexDesc, &resourceData, &indexBuffers[0] );
	}

	if (numOfInsts > 0)
	{
		instanceCount[0] = numOfInsts;
		D3D11_BUFFER_DESC instanceBufferDesc;
		ZeroMemory( &instanceBufferDesc, sizeof( instanceBufferDesc ) );
		instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		instanceBufferDesc.ByteWidth = InstbyteSize * numOfInsts;
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags = 0;
		instanceBufferDesc.MiscFlags = 0;
		instanceBufferDesc.StructureByteStride = 0;

		resourceData.pSysMem = instances;
		d3dDevice->CreateBuffer( &instanceBufferDesc, &resourceData, &instanceBuffers[0]);

		instStride = InstbyteSize;
	}
}
void RawVerts::Load(ID3D11Device* d3dDevice, std::vector<std::pair<unsigned int, unsigned int>>* arry,
											void* verts, unsigned int vertbyteSize, unsigned int numOfVerts,
											unsigned short** indices, unsigned short numOfBuffs, unsigned int numOfInds,
											void* instances, unsigned int InstbyteSize, unsigned int numOfInsts)
{
	//Clear();
	D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory( &resourceData, sizeof( resourceData ) );
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	if (numOfVerts > 0)
	{
		for (UINT i = 0; i<numVBuffers; i++){
			if( vertexBuffers[i] ) vertexBuffers[i]->Release( );
				vertexBuffers[i] = 0;
		}

		unsigned int vertCounter = numOfVerts;
		numVBuffers = numOfVerts/MAXVERT;
		numVBuffers++;
		vertexCount = new unsigned int[numVBuffers];
		for(unsigned int i = 0; i<numVBuffers; i++)
		{
			vertCounter = vertCounter-(i*MAXVERT);
			if (vertCounter > MAXVERT)
				vertexCount[i] = MAXVERT;
			else 
				vertexCount[i] = vertCounter;
			D3D11_BUFFER_DESC vertexDesc;
			ZeroMemory( &vertexDesc, sizeof( vertexDesc ) );
			vertexDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexDesc.ByteWidth = vertbyteSize * numOfVerts;
			vertexDesc.CPUAccessFlags = 0;

			unsigned int parseSize = sizeof(vertbyteSize)/sizeof(char);
			char* dI = (char*)verts;
			resourceData.pSysMem = verts;
			d3dDevice->CreateBuffer( &vertexDesc, &resourceData, &vertexBuffers[i] );

			stride = vertbyteSize;
		}
	}

	if (numOfInds > 0)
	{
		for (UINT i = 0; i<numIBuffers; i++)
		{
			if( indexBuffers[i] )
			{
				indexBuffers[i]->Release( );
				indexBuffers[i] = NULL;
			}
		}
		if (numOfBuffs != numIBuffers){
			numIBuffers = numOfBuffs;
			indexBuffers.resize(numOfBuffs);
			delete[] indicesCount;
			indicesCount = new unsigned int[numOfBuffs];
			std::pair<char,float> gpuInsert('b',numIBuffers);
			GlobalData::gbMap->erase('b');
			GlobalData::gbMap->insert(gpuInsert);
		}
		unsigned int indicesCounter = numOfInds;
		//numIBuffers = numOfInds/MAXINDS;
		//numIBuffers++;
		for(unsigned int i = 0; i<numOfBuffs; i++)
		{
			//indicesCount[i] = vertexCount[i]*indStride;
			/*if (indicesCounter > indStride)
			{
				indicesCount[i] = indStride;
				indicesCounter = indicesCounter-indStride;
				//indicesCounter = indicesCounter-(i*indStride);
			}
			else
				indicesCount[i] = indicesCounter;
			*/
			//std::pair<unsigned int, unsigned int> p;// = arry->at(i);
			indicesCount[i] = (arry->at(i).second/2)*(6*6);
			D3D11_BUFFER_DESC indexDesc;
			ZeroMemory( &indexDesc, sizeof( indexDesc ) );
			indexDesc.Usage = D3D11_USAGE_DEFAULT;
			indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexDesc.ByteWidth = sizeof(unsigned short) * indicesCount[i];
			indexDesc.CPUAccessFlags = 0;

			//unsigned int parseSize = sizeof(indbyteSize)/sizeof(char);
			//char* dI = (char*)indices;
			resourceData.pSysMem = indices[i];
			indexBuffers[i] = NULL;
			d3dDevice->CreateBuffer( &indexDesc, &resourceData, &indexBuffers[i] );
		}
	}

	if (numOfInsts > 0)
	{
		for (UINT i = 0; i<numInstBuffers; i++){
			if( instanceBuffers[i] ) instanceBuffers[i]->Release( );
				instanceBuffers[i] = 0;
		}

		unsigned int instsCounter = numOfInsts;
		numInstBuffers = numOfInsts/MAXINST;
		numInstBuffers++;
		instanceCount = new unsigned int[numInstBuffers];
		for(unsigned int i = 0; i<numInstBuffers; i++)
		{
			instsCounter = instsCounter-(i*MAXINST);
			if (instsCounter > MAXINST)
				instanceCount[i] = MAXINST;
			else 
				instanceCount[i] = instsCounter;
			D3D11_BUFFER_DESC instanceBufferDesc;
			ZeroMemory( &instanceBufferDesc, sizeof( instanceBufferDesc ) );
			instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			instanceBufferDesc.ByteWidth = InstbyteSize * numOfInsts;
			instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			instanceBufferDesc.CPUAccessFlags = 0;
			instanceBufferDesc.MiscFlags = 0;
			instanceBufferDesc.StructureByteStride = 0;
			resourceData.pSysMem = instances;
			d3dDevice->CreateBuffer( &instanceBufferDesc, &resourceData, &instanceBuffers[i] );
			instStride = InstbyteSize;
		}
	}
}

void RawVerts::Clear()
{
	delete[] vertexCount;
	for (UINT i = 0; i<numVBuffers; i++){
		if( vertexBuffers[i] ) vertexBuffers[i]->Release( );
		vertexBuffers[i] = 0;
	}

	delete[] indicesCount;
	for (UINT i = 0; i<numIBuffers; i++){
		if( indexBuffers[i] ) indexBuffers[i]->Release( );
		indexBuffers[i] = 0;
	}

	delete[] instanceCount;
	for (UINT i = 0; i<numInstBuffers; i++){
		if( instanceBuffers[i] ) instanceBuffers[i]->Release( );
		instanceBuffers[i] = 0;
	}
	/*if (vertexBuffer_) vertexBuffer_->Release();
	if (indexBuffer_) indexBuffer_->Release();
	if (instanceBuffer_) instanceBuffer_->Release();
	instanceCount = 0;
	vertexCount = 0;
	indicesCount = 0;*/

}

void RawVerts::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	//Here we check to if we're rendering instanced data or not
	//NOT INSTANCED
	if (instanceCount[0] == 0)
	{
		if(vertexCount[0] != 0)
		{
			for(int i = 0; i < numVBuffers; i++)
			{
				unsigned int offset = 0;
				d3dContext->IASetVertexBuffers( i, 1, &vertexBuffers[i], &stride, &offset );
				d3dContext->IASetIndexBuffer( indexBuffers[i], DXGI_FORMAT_R16_UINT, 0 );
				if (isLineList)
					d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );	
				else
					d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

				ID3DX11EffectTechnique* multiTexTechnique;
				multiTexTechnique = shaders[0].effect->GetTechniqueByName( effectName );

				D3DX11_TECHNIQUE_DESC techDesc;
				multiTexTechnique->GetDesc( &techDesc );

				for( unsigned int p = 0; p < techDesc.Passes; p++ )
				{
					ID3DX11EffectPass* pass = multiTexTechnique->GetPassByIndex( p );

					if( pass != 0 )
					{
						pass->Apply( 0, d3dContext );
						d3dContext->DrawIndexed( indicesCount[0], 0, 0 );
					}
				}
			}
		}
	}

	else
	//INSTANCED
	{
		unsigned int strideInst[2];
		unsigned int offset[2] = {0,0};
		strideInst[0] = stride;
		strideInst[1] = instStride;

		ID3D11Buffer* bufferPointers[2];
		bufferPointers[0] = vertexBuffers[0];	
		bufferPointers[1] = instanceBuffers[0];
		d3dContext->IASetVertexBuffers( 0, 2, bufferPointers, strideInst, offset );
		d3dContext->IASetIndexBuffer( indexBuffers[0], DXGI_FORMAT_R16_UINT, 0 );
		d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		ID3DX11EffectTechnique* multiTexTechnique;
		multiTexTechnique = shaders[0].effect->GetTechniqueByName( effectName );

		D3DX11_TECHNIQUE_DESC techDesc;
		multiTexTechnique->GetDesc( &techDesc );

		for( unsigned int p = 0; p < techDesc.Passes; p++ )
		{
			ID3DX11EffectPass* pass = multiTexTechnique->GetPassByIndex( p );

			if( pass != 0 )
			{
				pass->Apply( 0, d3dContext );
				d3dContext->DrawIndexedInstanced(indicesCount[0],instanceCount[0],0,0,0);
			}
		}
	}
}

void RawVerts::RenderRaw(ID3D11DeviceContext* d3dContext)
{
	//Here we check to if we're rendering instanced data or not
	//NOT INSTANCED
	if (instanceCount[0] == 0)
	{
		if(vertexCount[0] != 0)
		{
			for(int i = 0; i < numVBuffers; i++)
			{
				unsigned int offset = 0;
				d3dContext->IASetVertexBuffers( i, 1, &vertexBuffers[i], &stride, &offset );
				d3dContext->IASetIndexBuffer( indexBuffers[i], DXGI_FORMAT_R16_UINT, 0 );
				if (isLineList)
					d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );	
				else
					d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
				d3dContext->DrawIndexed( indicesCount[0], 0, 0 );
			}
		}
	}

	else
	//INSTANCED
	{
		unsigned int strideInst[2];
		unsigned int offset[2] = {0,0};
		strideInst[0] = stride;
		strideInst[1] = instStride;

		ID3D11Buffer* bufferPointers[2];
		bufferPointers[0] = vertexBuffers[0];	
		bufferPointers[1] = instanceBuffers[0];
		d3dContext->IASetVertexBuffers( 0, 2, bufferPointers, strideInst, offset );
		d3dContext->IASetIndexBuffer( indexBuffers[0], DXGI_FORMAT_R16_UINT, 0 );
		d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		d3dContext->DrawIndexedInstanced(indicesCount[0],instanceCount[0],0,0,0);
	}
}