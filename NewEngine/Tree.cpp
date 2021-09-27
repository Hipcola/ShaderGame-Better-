#include"Tree.h"
#include"GlobalData.h"
#include"utility.h"

//REMOVE ONLY FOR DEBUG STRING OUTPUT IN RUNCPUBUILD
//#include"utility.h"
//

Tree::Tree()
{
	base = XMFLOAT4(0.0f,0.0f,0.0f,1.0f);
	setDefaults();
};

Tree::Tree(XMFLOAT4 b)
{
	base = b;
	setDefaults();
};

Tree::~Tree( )
{
	for ( unsigned int b = 0; b<numBuffers;b++ )
	{
		vertices[b].clear();
		indices[b].clear();
		multiBuffOffsets[b].clear();
	}
	vertices.clear();
	indices.clear();
	multiBuffOffsets.clear();
	delete treeBuilder;
}

void Tree::setDefaults()
{
	treeBuilder = NULL;
	isWire = true;
	isGPUBased = false;
	setBuffers = false;

	numBuffers = 0;
	newNumBuffers = 1;
	vecBuffCounter = 1;
	buffOffSet = 0;
	resultVers.resize(1);
	resultVersSRV.resize(1);
	resultVersUAV.resize(1);
	resultVers[0] = NULL;
	resultVersSRV[0] = NULL;
	resultVersUAV[0] = NULL;
	resultInds.resize(1);
	resultIndsSRV.resize(1);
	resultIndsUAV.resize(1);
	resultInds[0] = NULL;
	resultIndsSRV[0] = NULL;
	resultIndsUAV[0] = NULL;
	vertices.resize(1);
	indices.resize(1);
	vecsinds.resize(1);
	multiBuffOffsets.resize(1);
	Tree::Clear();
}

void Tree::AddBranch(XMFLOAT4 vec, DWORD indice)
{
	numVerts++;
	sectionNumVerts++;
	vecBuffCounter++;
	//if (numInds == 135528)
	// unsigned int adf = 324;
	    
	if (sectionNumVerts == SPLITPOINT)
	{   
		//vecBuffCounter = 0;
		sectionNumVerts = 0;
		sectionNumInds = 0;
	    newNumBuffers++;
		//if (newNumBuffers > 6 && numInds == 135528)
			
		vertices.resize(newNumBuffers);
		indices.resize(newNumBuffers);
		multiBuffOffsets.resize(newNumBuffers);
		sectionNumVerts++;
		//vecBuffCounter++;
	}
	unsigned int buffP = newNumBuffers-1;
	if (indice<= buffP*(SPLITPOINT-2) && indice != 0)
	{
		unsigned int indiceLocation = indice/(SPLITPOINT-1);
		unsigned int offset = indice-((SPLITPOINT-1)*indiceLocation);//(indice/(SPLITPOINT-2))+1;//-((buffP-1)*(SPLITPOINT-2));//indice*buffP;//indice%(SPLITPOINT-2);
		//unsigned int offset = 0;
		for(UINT i=0; i<multiBuffOffsets.size();i++)
		{
			if (i < indiceLocation)
			{
				offset += multiBuffOffsets[i].size();
				if (offset > (SPLITPOINT-2))
				{
					indiceLocation++;
					offset = 0 + (offset-(SPLITPOINT-1));
					//test = 0;
				}
			}
			else if ( indiceLocation == i )
			{
				unsigned int msize = multiBuffOffsets[i].size();
				//vector<unsigned int> mbuff = multiBuffOffsets[i];
				bool flag = false;
				for(UINT m = 0; m<msize;m++)
				{
					if (multiBuffOffsets[i][m] <= offset)
					{
						offset++;//= multiBuffOffsets[i][m];
						//offset += multiBuffOffsets[i][m];
						if (offset > (SPLITPOINT-2) && !flag)
						{
							indiceLocation++;
							//offset = 0;
							flag = true;
							//test = 0;
						}
					}
					else
						break;
				}
				if (flag)
					offset = 0 + (offset-(SPLITPOINT-1));
			}
		}
		vertices[buffP].push_back(vertices[indiceLocation].at(offset));
		multiBuffOffsets[buffP].push_back(vertices[buffP].size()-1);
		//buffOffSet += vertices[buffP].size()-1;
		buffOffSet++;//= vertices[buffP].size()-1;
		numVerts++;
		sectionNumVerts++;
		indices[buffP].push_back(sectionNumVerts-2);
	}
	else
	{
		if (indice > SPLITPOINT-2)
		{
		  //indices[buffP].push_back(indice-(buffP*(SPLITPOINT-2)));
			//unsigned int test1 = buffP*(SPLITPOINT-1);
			//unsigned int test2 = indice-(test1-buffOffSet);
			indices[buffP].push_back(indice-((buffP*(SPLITPOINT-1))-buffOffSet));
		}
		else
			indices[buffP].push_back(indice);
	}
	vertices[buffP].push_back(vec);
	//indices[buffP].push_back(indice/*sectionNumVerts-2*/);
	indices[buffP].push_back(sectionNumVerts-1);
	numInds++;
	numInds++;
	sectionNumInds++;
	sectionNumInds++;
}

void Tree::Build(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11ShaderResourceView* inVers, ID3D11ShaderResourceView* inInds )
{
	//#TODO V Remove this V
	setPosition(XMFLOAT3(vertices[0][0].x,vertices[0][0].y,vertices[0][0].z));
	setRotation(&XMMatrixRotationRollPitchYaw( 0.0f ,0.0f, 0.0f ));
	if (!setBuffers)
	{
		setBuffers = true;
	}
	if (isWire)
	{
		RawVerts::Load(d3dDevice, 
			vertices.data(), sizeof(XMFLOAT3), numVerts, 
			indices.data(), sizeof(WORD), numInds, 
			NULL, NULL, 0);
	}else
	{
		//if(!isGPUBased)
		//{
		//	runCPUBuild(d3dDevice);
		//}else{
			if (treeBuilder == NULL)
			{
				treeBuilder = new GPUTreeBuilder;
				treeBuilder->Load(d3dDevice,SIDES);
			}
			runGPUBuild(d3dDevice, d3dContext, inVers, inInds);
		//}
	}
	//string a = "Built\n";
	//OutputDebugString(a.c_str());
}

void Tree::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext,float delta, void* data)
{
}

XMFLOAT3* Tree::buildTrunkVerts(int position)
{
	//Build tree trunk verts
	XMFLOAT3* trunkVerts = new XMFLOAT3[SIDES];
	float width = 0;
	int widthSearch = position;
	while (width == 0){
		width = vertices[0].at(widthSearch).w;
		widthSearch++;
	}
	float angle = (360.f / SIDES)*(3.14159f / 180);
	float angleI = angle;
	for (int i=0; i<SIDES;i++)
	{
		float x = width*cos(angleI);
		float y = width*sin(angleI);
		trunkVerts[i] = XMFLOAT3(x,0,y);
		angleI = angleI + angle;
	}
	return trunkVerts;
}

void Tree::runCPUBuild(ID3D11Device* d3dDevice)
{
	//Create verts for the 3d tree
	if (vertices.size() <= 1)
	{
		//Causes crash
		//TODO : Investigate. This indicesCount = 0 is a patchwork fix
		//RawVerts::Clear();
		this->indicesCount = 0;
		RawVerts::Load(d3dDevice, 0,
		NULL, NULL, 0,
		NULL, NULL, 0,
		NULL, NULL, 0);
	}
	else{
		VertexPosNormTexRV* vers;
		int vertsTotal = numVerts*SIDES;
		vers = new VertexPosNormTexRV[vertsTotal];
		int outcount = 0;
		int texIncrease = 1;

		//int indsSize = indices.size();
		XMFLOAT3* trunkVerts;// = new XMFLOAT3[sides];
		float* texcoord = new float[SIDES];
		float texCount = 1.0f/(SIDES-1);
		float count = 0;
		for (int a = 0;a<SIDES;a++){
			texcoord[a] = count; 
			count += texCount;
		}
		//Build base
		trunkVerts = buildTrunkVerts(0);
		for (int a = 0;a<SIDES;a++){
			vers[outcount].pos = XMFLOAT3(vertices[0].at(0).x + trunkVerts[a].x, 
										  vertices[0].at(0).y + trunkVerts[a].y, 
										  vertices[0].at(0).z + trunkVerts[a].z);
			vers[outcount].norm = trunkVerts[a];
			vers[outcount].tex0 = XMFLOAT2(texcoord[a],0);
			outcount++;
		}

		for (int i=1; i<numInds;i=i+2){
			XMVECTOR v = XMVectorSet(0,1,0,0);
			WORD next = indices[0].at(i);
			WORD last = indices[0].at(i-1);

			trunkVerts = buildTrunkVerts(next);

			//Rotate the verts accordingly
			XMVECTOR V = XMVectorSet(vertices[0].at(next).x-vertices[0].at(last).x,vertices[0].at(next).y-vertices[0].at(last).y,vertices[0].at(next).z-vertices[0].at(last).z,0);
			//float v1 = *XMVector3Length(v).m128_f32;
			XMVECTOR axisV = XMVector3Cross(v,V);
			axisV = XMVector3Normalize(axisV);
			XMFLOAT4 axis;
			XMStoreFloat4(&axis,axisV);
			float v1 = *XMVector3Length(axisV).m128_f32;
			if (v1 >0)
			{
				V = XMVector3Normalize(V);
				//V = XMVector3AngleBetweenVectors(v,V);
				V = XMVector3AngleBetweenNormals(v,V);
		        //V = XMVector3Dot(v,V);
				//V = XMVectorACos(V);
				XMMATRIX m = XMMatrixRotationAxis(axisV,XMVectorGetX(V));
				for (int a = 0;a<SIDES;a++)
				{
					v =  XMVector3Transform(XMVectorSet(trunkVerts[a].x,trunkVerts[a].y,trunkVerts[a].z,0),m);
					trunkVerts[a] = XMFLOAT3(XMVectorGetX(v),XMVectorGetY(v),XMVectorGetZ(v));
				}
			}

			//store in the array
			for (int a = 0;a<SIDES;a++){
				vers[outcount].pos = XMFLOAT3(vertices[0].at(next).x + trunkVerts[a].x, 
											vertices[0].at(next).y + trunkVerts[a].y, 
											vertices[0].at(next).z + trunkVerts[a].z);
				vers[outcount].norm = trunkVerts[a];
				vers[outcount].tex0 = XMFLOAT2(texcoord[a],(vers[last*SIDES].tex0.y+1));
				outcount++;
			}
			texIncrease++;
		}

		RawVerts::Load(d3dDevice,
			vers, sizeof(VertexPosNormTexRV), vertsTotal, 
			NULL, NULL, 0, 
			NULL, NULL, 0);
		pair<char,float> vertInsert('v',vertsTotal);
		GlobalData::gbMap->erase('v');
		GlobalData::gbMap->insert(vertInsert);

		delete[] trunkVerts;
		delete[] vers;
		delete[] texcoord;
	}
}

void Tree::runGPUBuild(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11ShaderResourceView* inVers, ID3D11ShaderResourceView* inInds)
{
	if(numVerts > 1)
	{
		treeBuilder->buildTree(d3dContext, inVers, inInds, resultVersUAV[0], resultIndsUAV[0], numVerts, numInds, d3dDevice);

		///*
		ID3D11Buffer* debugbuf6 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, resultInds[0] );
		D3D11_MAPPED_SUBRESOURCE MappedResource6;
		d3dContext->Map( debugbuf6, 0, D3D11_MAP_READ, 0, &MappedResource6 );
		unsigned int* dSize2 = (unsigned int*)MappedResource6.pData;	
		d3dContext->Unmap( debugbuf6, 0 );
		//*/
		///*
		ID3D11Buffer* debugbuf5 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, resultVers[0] );
		D3D11_MAPPED_SUBRESOURCE MappedResource5;
		d3dContext->Map( debugbuf5, 0, D3D11_MAP_READ, 0, &MappedResource5 );
		VertexPosNormTexRV* dSize = (VertexPosNormTexRV*)MappedResource5.pData;	
		d3dContext->Unmap( debugbuf5, 0 );
		//*/
	}
}

void Tree::updateIndices(ID3D11Device* d3dDevice)
{
	if(!isGPUBased)
	{
		vecsinds.clear();

		//Indices for 3d tree
		int indicesTotal = (vecBuffCounter-1)*(SIDES*6);
		if (newNumBuffers != numBuffers)
			vecsinds.reserve(newNumBuffers);
		WORD** indsArry = new WORD*[newNumBuffers];
		for (int indBuffCount = 0; indBuffCount<newNumBuffers; indBuffCount++)
		{
			unsigned int vecsRepByThisBuff = vertices[indBuffCount].size();//(thisIndBuffTotal/(SIDES*6))+1;
			unsigned int indsRepByThisBuff = indices[indBuffCount].size();//(vecsRepByThisBuff-1)*2;
			std::pair<unsigned int, unsigned int> p = std::make_pair(vecsRepByThisBuff, indsRepByThisBuff);
			vecsinds.push_back(p);
			unsigned int thisIndBuffTotal = (indsRepByThisBuff/2)*(SIDES*6);
			WORD* inds = new WORD[thisIndBuffTotal];
			for (unsigned int i=0;i<indsRepByThisBuff;i+=2)
			{
				unsigned int upperVertCount = (indices[indBuffCount].at(i+1) * SIDES);
				unsigned int lowerVertCount = (indices[indBuffCount].at(i) * SIDES);
				unsigned int lastSecL = lowerVertCount;
				unsigned int lastSecU = upperVertCount;
				unsigned int b = (i/2)*(SIDES*6);
				unsigned int size = b+((SIDES-1)*6);
				for (;b<size;b=b+6)
				{
					inds[b]   = lowerVertCount;
					inds[b+1] = upperVertCount;
					inds[b+2] = lowerVertCount+1;
					inds[b+3] = upperVertCount;
					inds[b+4] = upperVertCount+1;
					inds[b+5] = lowerVertCount+1;
					upperVertCount++;
					lowerVertCount++;
				}
				inds[b]   = lowerVertCount;
				inds[b+1] = upperVertCount;
				inds[b+2] = lastSecL;
				inds[b+3] = upperVertCount;
				inds[b+4] = lastSecU;//(section*SIDES)+SIDES;
				inds[b+5] = lastSecL;
			}
			indsArry[indBuffCount] = inds;
		}
		RawVerts::Load(d3dDevice, &vecsinds,
		NULL, NULL, 0, 
		indsArry, newNumBuffers, (vecBuffCounter-1)*(SIDES*6)/*(numVerts-1)*(SIDES*6)*/,
		NULL, NULL, 0);

		/*
		//Update buffer for gpu implementation
		for ( unsigned int b = 0; b<numBuffers;b++ )
		{
			SAFE_RELEASE(resultVers[b]);
			SAFE_RELEASE(resultVersSRV[b]);
			SAFE_RELEASE(resultVersUAV[b]);

		}*/

		if (newNumBuffers != numBuffers)
		{
			numBuffers = newNumBuffers;
			resultVers.resize(numBuffers);
			resultVersSRV.resize(numBuffers);
			resultVersUAV.resize(numBuffers);
		}

		for ( unsigned int b = 0; b<numBuffers;b++ )
		{
			unsigned int numVs = vecsinds[b].first*SIDES;
			/*
			VertexPosNormTexRV* data = new VertexPosNormTexRV[numVs];
			for( UINT i = 0; i < numVs ; i++ )
			{
				data[i].pos = XMFLOAT3( 0, 0, 0 );
				data[i].tex0 = XMFLOAT2( 0, 0 );
				data[i].norm = XMFLOAT3( 0, 0, 0 );
			}*/

			SAFE_RELEASE(resultVers[b]);
			SAFE_RELEASE(resultVersSRV[b]);
			SAFE_RELEASE(resultVersUAV[b]);
			resultVers[b] = NULL;
			resultVersSRV[b] = NULL;
			resultVersUAV[b] = NULL;
			Utility::CreateStructuredBuffer(d3dDevice, sizeof(VertexPosNormTexRV), numVs, NULL/*data*/, &resultVers[b] );
			Utility::CreateBufferSRV(d3dDevice, resultVers[b], &resultVersSRV[b] );
			Utility::CreateBufferUAV(d3dDevice, resultVers[b], &resultVersUAV[b] );
			//delete [] data;
		}

		for ( unsigned int b = 0; b<numBuffers;b++ )
		{
			delete [] indsArry[b];
		}
		delete [] indsArry;
	}
	else
	{
		//newNumBuffers = (numVerts/SPLITPOINT);
		//if (newNumBuffers != numBuffers)
		//{
			numBuffers = newNumBuffers;
			resultVers.resize(numBuffers);
			resultVersSRV.resize(numBuffers);
			resultVersUAV.resize(numBuffers);
			resultInds.resize(numBuffers);
			resultIndsSRV.resize(numBuffers);
			resultIndsUAV.resize(numBuffers);
			for ( unsigned int b = 0; b<numBuffers;b++ )
			{
				//Needs to be found for gpu
				unsigned int numVs = numVerts*SIDES;
				unsigned int numIs = (numInds/2)*(SIDES*6);

				SAFE_RELEASE(resultVers[b]);
				SAFE_RELEASE(resultVersSRV[b]);
				SAFE_RELEASE(resultVersUAV[b]);
				resultVers[b] = NULL;
				resultVersSRV[b] = NULL;
				resultVersUAV[b] = NULL;
				Utility::CreateStructuredBuffer(d3dDevice, sizeof(VertexPosNormTexRV), numVs, NULL/*data*/, &resultVers[b] );
				Utility::CreateBufferSRV(d3dDevice, resultVers[b], &resultVersSRV[b] );
				Utility::CreateBufferUAV(d3dDevice, resultVers[b], &resultVersUAV[b] );

				if(numIs > 0)
				{
					SAFE_RELEASE(resultInds[b]);
					SAFE_RELEASE(resultIndsSRV[b]);
					SAFE_RELEASE(resultIndsUAV[b]);
					resultInds[b] = NULL;
					resultIndsSRV[b] = NULL;
					resultIndsUAV[b] = NULL;
					Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), numIs, NULL/*data*/, &resultInds[b] );
					Utility::CreateBufferSRV(d3dDevice, resultInds[b], &resultIndsSRV[b] );
					Utility::CreateBufferUAV(d3dDevice, resultInds[b], &resultIndsUAV[b] );
				}
			}
		//}
	}
}

#include"TexturePhongShader.h"

void Tree::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	ID3DX11EffectTechnique* multiTexTechnique;
	multiTexTechnique = shaders[0].effect->GetTechniqueByName( effectName );
	D3DX11_TECHNIQUE_DESC techDesc;
	multiTexTechnique->GetDesc( &techDesc );
	if (isLineList)
		d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );	
	else
		d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	ShaderDataTP d;
	d.viewMat = ((RenderData*)data)->viewMat;
	d.projMat = ((RenderData*)data)->projMat;
	d.worldMat = &XMLoadFloat4x4(&worldMat);
	XMFLOAT4* camP = ((RenderData*)data)->camPos;
	float camInP [] = {camP->x, camP->y, camP->z};
	d.camPos = camInP;
	d.inputType = 2;
	float lightCol [] = {lightColour.x,lightColour.y,lightColour.z,lightColour.w};
	d.lightCol = lightCol;
	float lightPos [] = {lightPosition.x,lightPosition.y,lightPosition.z};
	d.lightPos = lightPos;

	if (!isGPUBased)
	{
		if (indices.size() == 0) {}
		else
		{
			for( UINT i = 0; i < numBuffers ; i++ )
			{
				d.buffer = resultVersSRV[i];
				((TexturePhongShader*)&shaders[0])->preRender(d3dContext,&d);

				d3dContext->IASetIndexBuffer( indexBuffers[i], DXGI_FORMAT_R16_UINT, 0 );

				for( unsigned int p = 0; p < techDesc.Passes; p++ )
				{
					ID3DX11EffectPass* pass = multiTexTechnique->GetPassByIndex( p );

					if( pass != 0 )
					{
						pass->Apply( 0, d3dContext );
						d3dContext->DrawIndexed( indicesCount[i], 0, 0 );
					}
				}
				ID3D11ShaderResourceView* srNULL[1] = { NULL };
				d3dContext->VSSetShaderResources(1,1,srNULL);
			}
		}
	}else{
		for( UINT i = 0; i < numBuffers ; i++ )
		{
			d.buffer = resultVersSRV[i];
			((TexturePhongShader*)&shaders[0])->preRender(d3dContext,&d);

			d3dContext->IASetIndexBuffer( resultInds[i], DXGI_FORMAT_R32_UINT, 0 );

			for( unsigned int p = 0; p < techDesc.Passes; p++ )
			{
				ID3DX11EffectPass* pass = multiTexTechnique->GetPassByIndex( p );

				if( pass != 0 )
				{
					pass->Apply( 0, d3dContext );
					d3dContext->DrawIndexed( (numInds/2)*(SIDES*6), 0, 0 );
				}
			}
			ID3D11ShaderResourceView* srNULL[1] = { NULL };
			d3dContext->VSSetShaderResources(1,1,srNULL);
		}
	}
}

void Tree::Clear()
{
	if (!isGPUBased)
	{
		for ( unsigned int b = 0; b<numBuffers;b++ )
		{
			vertices[b].clear();
			indices[b].clear();
			multiBuffOffsets[b].clear();
		}
		//vertices.clear(); 
		//indices.clear();
		multiBuffOffsets.clear();
		vertices[0].push_back(base);
		//numBuffers = 1;
		buffOffSet = 0;
		newNumBuffers = 1;
		numVerts=1;
		vecBuffCounter=1;
		sectionNumVerts=1;
		numInds =0;
		sectionNumInds=0;
	}
}

XMFLOAT4 Tree::getBase()
{
	return base;
}

void Tree::setNumVerts(unsigned int vers)
{
	numVerts = vers;
	numInds = vers*2;
}

unsigned int Tree::getNumVerts()
{
	return ((numVerts-1)*2)*SIDES;
}

unsigned int Tree::getNumInds()
{
	return (numVerts-1)*(SIDES*6);
}

std::vector<XMFLOAT4>* Tree::getVerts()
{
	return &vertices[0];
}

/*std::vector<WORD>* Tree::getIndices()
{
	return &indices;
}*/