#include"TreeSystem.h"
#include"GPUTreeBuilder.h"
#include"GlobalData.h"
#include<dinput.h>

//debugging
#include<time.h>

#define KEYDOWN( name, key ) ( name[key] & 0x80 )
#define BUTTONDOWN(device, key) ( device.rgbButtons[key] & 0x80 )

TreeSystem::TreeSystem()
{
	//isGrowing = false;
	isGrowing = false;
	isPaused = false;
	totalTime = 1.f;//6.5f;
	startTime = 0;
	renderTree = true;
	renderLeaves = true;
	treeAltered = false;
	pair<char,float> gpuInsert('g',1);
	GlobalData::gbMap->erase('g');
	GlobalData::gbMap->insert(gpuInsert);
	pair<char,float> pInsert ('p',0);
	GlobalData::gbMap->erase('p');
	GlobalData::gbMap->insert(pInsert);
	pair<char,float> tInsert ('t',0);
	GlobalData::gbMap->erase('t');
	GlobalData::gbMap->insert(tInsert);
	pair<char,float> bInsert ('b',1);
	GlobalData::gbMap->erase('b');
	GlobalData::gbMap->insert(bInsert);
};

TreeSystem::~TreeSystem( )
{
	delete treeBuilder;
	delete lsysBuilder;
	delete trees;
	delete leaves;
}

void TreeSystem::setTree(unsigned int treeSetting, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext)
{
	currLSys = treeSetting;
	trees->Clear();
	leaves->EmptyLeaves();

	if (trees->isGPUBased == false)
	{
		if (tropisms[currLSys].first == 0)
			parser->clearTropism();
		else
			parser->setTropism(tropisms[currLSys].second,tropisms[currLSys].first);
		parser->parse(trees, leaves, lSys[currLSys], 1.0f);
	}
	else
	{
		//getVers will check to parse to l-system
		//This shouldn't be here, especially update indacies. We need build a new system for trees first run on the gpu
		lSys[currLSys]->getVers(d3dDevice,d3dContext);
		trees->numVerts = lSys[currLSys]->vertCount;
		trees->numInds = lSys[currLSys]->indCount;
		//trees->updateIndices(d3dDevice);
	}
	pair<char,float> stringInsert ('s',lSys[currLSys]->ldata->size()/*lSys[currLSys]->getLength()*/);
	GlobalData::gbMap->erase('s');
	GlobalData::gbMap->insert(stringInsert);
	treeAltered = true;
}

void TreeSystem::clearTrees()
{
	int whyarewehere=0;
	/*for (int i = 0; i<numTrees;i++)
	{
		trees[i]->Clear();
	}*/
}

void TreeSystem::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data)
{
	BasicInputData* d = (BasicInputData*)data;

	//Light Movement
	float lightForwardBack = 0.0f;
	float lightLeftRight = 0.0f;

	float speed = 1.0f*delta;
	if( KEYDOWN( d->prevKeyboardKeys, DIK_UP ) )
	{ 
		lightLeftRight += speed;
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_DOWN ) )
	{ 
		lightLeftRight -= speed;
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_LEFT ) )
	{ 
		lightForwardBack -= speed;
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_RIGHT ) )
	{ 
		lightForwardBack += speed;
	}

	XMFLOAT3 p = trees->lightPosition;
	p.x += lightForwardBack;
	p.z += lightLeftRight;
	trees->lightPosition = p;

	if( KEYDOWN( d->prevKeyboardKeys, DIK_1 ) && !KEYDOWN( d->keyboardKeys, DIK_1 ))
	{
		if (currLSys != 0 && !isGrowing)
		{
			setTree(0,d3dDevice,d3dContext);
			trees->updateIndices(d3dDevice);
			leaves->Build(d3dDevice);
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_2 ) && !KEYDOWN( d->keyboardKeys, DIK_3 ))
	{
		if (currLSys != 1  && !isGrowing)
		{
			setTree(1,d3dDevice,d3dContext);
			trees->updateIndices(d3dDevice);
			leaves->Build(d3dDevice);
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_3 ) && !KEYDOWN( d->keyboardKeys, DIK_3 ))
	{
		if (currLSys != 2  && !isGrowing)
		{
			setTree(2,d3dDevice,d3dContext);
			trees->updateIndices(d3dDevice);
			leaves->Build(d3dDevice);
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_L ) && !KEYDOWN( d->keyboardKeys, DIK_L ))
	{
		if (renderLeaves == true && renderTree == true)
			renderLeaves = false;
		else if (renderTree == true)
		{
			renderTree = false;
			renderLeaves = true;
		}
		else
		{
			renderLeaves = true;
			renderTree = true;
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_I ) && !KEYDOWN( d->keyboardKeys, DIK_I ))
	{
		if (isGrowing){}
		else
		{
			//Finding the iteration time
			clock_t t;
			float duration;
			t = clock();
			lSys[currLSys]->iterate(totalTime);
			setTree(currLSys,d3dDevice,d3dContext);
			trees->updateIndices(d3dDevice);
			leaves->Build(d3dDevice);
			duration = (clock() - t);

			wstring a = L"iteration num:";
			a.append(Utility::convertFloatW(lSys[currLSys]->getIterations()));
			a.append(L" Time taken:");
			a.append(Utility::convertFloatW(duration));
			a.append(L"\n");
			OutputDebugString(a.c_str());
			parser->setParseGrowthData(lSys[currLSys]->getIterations());
			pair<char,float> stringInsert ('s',lSys[currLSys]->getLength());
			GlobalData::gbMap->erase('s');
			GlobalData::gbMap->insert(stringInsert);
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_G ) && !KEYDOWN( d->keyboardKeys, DIK_G ))
	{
		if (isGrowing || isPaused){}
		else{
			//testingClock = clock();
			isGrowing = true;
			numOfParses = 0;
			currLSize = 0;
			testingTimesBuild.clear();
			testingTimesParse.clear();
			testingTimesRw.clear();
			//deltaTime = 0;
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_C ) && !KEYDOWN( d->keyboardKeys, DIK_C ))
	{
		if (isGrowing || isPaused){}
		else{
			numOfParses = 0;
			currLSize = 0;
			testingTimesBuild.clear();
			testingTimesParse.clear();
			testingTimesRw.clear();
			lSys[currLSys]->reset();
			setTree(currLSys,d3dDevice,d3dContext);
			trees->updateIndices(d3dDevice);
			leaves->Build(d3dDevice);
		}
	}

	if( KEYDOWN( d->prevKeyboardKeys, DIK_N ) && !KEYDOWN( d->keyboardKeys, DIK_N ))
	{
		if (trees->isGPUBased == true)
		{
			trees->setShaderTechniqueName("TexturePhong");
			trees->isGPUBased = false;
			pair<char,float> gpuInsert ('g',0);
			GlobalData::gbMap->erase('g');
			GlobalData::gbMap->insert(gpuInsert);
		}
		else
		{
			trees->setShaderTechniqueName("TexturePhongBuffer");
			trees->isGPUBased = true;
			pair<char,float> gpuInsert ('g',1);
			GlobalData::gbMap->erase('g');
			GlobalData::gbMap->insert(gpuInsert);
		}
	}

	if(isGrowing || isPaused)
	{
		if( KEYDOWN( d->prevKeyboardKeys, DIK_P ) && !KEYDOWN( d->keyboardKeys, DIK_P ))
		{
			if (isPaused)
				isPaused = false;
			else
				isPaused = true;
		}else if(!isPaused)
		{
			if (startTime != totalTime)
			{
				startTime = startTime + delta;
				if (startTime>totalTime)
					startTime = totalTime;

				trees->Clear();
				leaves->EmptyLeaves();
				clock_t t = clock();
				/*if (numOfParses >= 1)
				{
					float fuasdf = 324;
				}*/
				if (trees->isGPUBased == true)
					lSys[currLSys]->iterateGPU(d3dDevice,d3dContext,delta);
				else
					lSys[currLSys]->iterate(delta);
				float duration = clock() - t;
				testingTimesRw.push_back(duration);
				pair<char,float> tInsert('t',startTime);
				GlobalData::gbMap->erase('t');
				GlobalData::gbMap->insert(tInsert);

				bool test = false;
				if (GlobalData::gbMap->at('s') != lSys[currLSys]->ldata->size())
				{
					pair<char,float> stringInsert('s',lSys[currLSys]->ldata->size());
					GlobalData::gbMap->erase('s');
					GlobalData::gbMap->insert(stringInsert);
					test = true;
				}
				numOfParses++;
				//bool test = false;
				t = clock();
				//parser->parse(trees, leaves, lSys[currLSys], 1.0f/*startTime/totalTime*/);
				duration = clock() - t;
				testingTimesParse.push_back(duration);
				if (test)
					trees->updateIndices(d3dDevice);
				t = clock();
				leaves->Build(d3dDevice);
				if (trees->numVerts != lSys[currLSys]->vertCount)
				{
					trees->numVerts = lSys[currLSys]->vertCount;
					trees->numInds = lSys[currLSys]->indCount;
					trees->updateIndices(d3dDevice);
				}
				//trees->getNumInds(lSys[currLSys]->get
				trees->Build(d3dDevice, d3dContext, lSys[currLSys]->getVers(d3dDevice,d3dContext), lSys[currLSys]->getInds());
				duration = clock()-t;
				testingTimesBuild.push_back(duration);
			}
			else{
				//testingDuration = (clock() - testingClock);
				float avgRW = 0;
				float avgP = 0;
				float avgB = 0;
				for (int i=0;i<testingTimesBuild.size();i++)
				{
					avgRW += testingTimesRw[i];
					avgP += testingTimesParse[i];
					avgB += testingTimesBuild[i];
				}
				avgRW = avgRW/testingTimesBuild.size();
				avgP = avgP/testingTimesBuild.size();
				avgB = avgB/testingTimesBuild.size();
				std::wstring a = L"RW = ";
				a.append(Utility::convertFloatW(avgRW));
				a.append(L"\n");
				OutputDebugString(a.c_str());
				a = L"Parse = ";
				a.append(Utility::convertFloatW(avgP));
				a.append(L"\n");
				OutputDebugString(a.c_str());
				a = L"Build = ";
				a.append(Utility::convertFloatW(avgB));
				a.append(L"\n");
				OutputDebugString(a.c_str());
				isGrowing = false;
				startTime = 0;
			}
		}
	}
	if (treeAltered)
	{
		trees->Build(d3dDevice,d3dContext,lSys[currLSys]->getVers(d3dDevice,d3dContext),lSys[currLSys]->getInds());
		treeAltered = false;
	}
}

void TreeSystem::Load(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, LoadDataTreeSystem* data)
{
	//Setting up LSystems
	currLSys = 0;
	lSys = new LSystem*[data->numSystems];
	this->lsysBuilder = new LSystemGPU();
	this->lsysBuilder->Load(d3dDevice);
	for (int i = 0; i<data->numSystems;i++)
	{
		lSys[i] = new LSystem(data->lsystemPointer[i]);
		lSys[i]->gpuLIterator = this->lsysBuilder;
	}

	//Setting up trees
	//numTrees = data->numSystems;
	trees = new Tree(XMFLOAT4(0.0f,0.0f,0.0f,0.03f));
	this->treeBuilder = new GPUTreeBuilder();
	this->treeBuilder->Load(d3dDevice,SIDES);
	trees->treeBuilder = this->treeBuilder;
	trees->isGPUBased = true;
	trees->isWire = false;
	trees->lightPosition = XMFLOAT3(0.0f,0.5f,0.0f);
	trees->passInShader(shaders);
	trees->setShaderTechniqueName("TexturePhongBuffer");

	tropisms = data->tropisms;
	parser = data->parserPointer;
	parser->setParseGrowthData(lSys[currLSys]->getIterations());

	leaves = new Leaves(0.03f);
	leaves->passInShader(data->leafShader);
	leaves->setShaderTechniqueName("TextureRotatedInstanced");
	leaves->SetBasicLeafGeo(d3dDevice);

	setTree(0,d3dDevice,d3dContext);
	trees->updateIndices(d3dDevice);
	leaves->Build(d3dDevice);
}

void TreeSystem::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	if (renderTree)
		trees->Render(d3dContext,data);
	if (renderLeaves)
		leaves->Render(d3dContext,data);
}
