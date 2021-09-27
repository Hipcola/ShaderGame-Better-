#include"ShaderGameV2.h"
#include"GlobalData.h"
#include<unordered_map>
#include<map>


//remove
/*
#define DONT_SAVE_VSGLOG_TO_TEMP
#define VSG_DEFAULT_RUN_FILENAME L"A2.vsglog"
#include <vsgcapture.h>
//*/

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

const float DEG2RAD = 3.14159f / 180;

ShaderGameV2::ShaderGameV2( ) 
{
	camera_2 = FPSCamera(SCREEN_WIDTH, SCREEN_HEIGHT);
	speed = 1;
	testingSwap = 1;
}

ShaderGameV2::~ShaderGameV2( )
{

}

bool ShaderGameV2::LoadContent( )
{
	numRObjects = 4;
	numShaders = 5;

	renderObjects = new RenderObject*[numRObjects];
	shaders = new Shader*[numShaders];

	/* Load Shaders */
	TexturePhongShader* tps = new TexturePhongShader;
	tps->Load(d3dDevice_);
	shaders[0] = tps;

	ColourShader* fcs = new ColourShader;
	fcs->Load(d3dDevice_);
	shaders[1] = fcs;

	SkyShader* sks = new SkyShader;
	sks->Load(d3dDevice_);
	shaders[2] = sks;

	Texture* tex = new Texture;
	tex->fileNameLoad(d3dDevice_,L"leaf2.dds");
	shaders[3] = tex;

	Text* text = new Text;
	text->fileNameLoad(d3dDevice_,"font.dds");
	shaders[4] = text;
	
	/* Load Objects */
	TreeSystem* t = new TreeSystem;

	const int numSystems = 1;
	LSystem* lSys [numSystems];// = new LSystem;
	for (int i = 0; i < numSystems; i++)
		lSys[i] = new LSystem();

	//LSParser parseObj;
    shared_ptr<LSParser> parser = shared_ptr<LSParser>(new LSParser()); //= parseObj;

	vector<RuleData> rules;
	unordered_map<char,float> table;
	pair<float,XMFLOAT3>* trops =  new pair<float,XMFLOAT3>[numSystems];

	RuleData r = RuleData("","","","");

	//lSys[0]->setString(&string("F(0.1,0.5,0)"));
	lSys[0]->setString(&string("A(0.5,0.5,1)"));
	//lSys[0]->setString(&string("D(0.5,0.5,1)-(d)D(0.5,0.5,1)&(a)D(0.5,0.5,1)"));

	//lSys[0]->setString(&string("Q"));																	// 11 end br
	//lSys[0]->setString(&string("F(0.5,0.5,0.5)[F(0.5,0.5,0.5)[F(0.5,0.5,0.5)F(0.5,0.5,0.5)][F(0.5,0.5,0.5)]F(0.5,0.5,0.5)]F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)[F(0.5,0.5,0.5)]"));
	//lSys[0]->setString(&string("F(0.5,0.5,0.5)[F(0.5,0.5,0.5)[F(0.5,0.5,0.5)F(0.5,0.5,0.5)][F(0.5,0.5,0.5)]F(0.5,0.5,0.5)]F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)[F(0.5,0.5,0.5)]F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)"));
	//lSys[0]->setString(&string("F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)[F(0.5,0.5,0.5)F(0.5,0.5,0.5)[F(0.5,0.5,0.5)F(0.5,0.5,0.5)][F(0.5,0.5,0.5)]F(0.5,0.5,0.5)]F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)[F(0.5,0.5,0.5)]F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)"));
	//lSys[0]->setString(&string("F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)F(0.5,0.5,0.5)\\(d)F(0.5,0.5,0.5)F(0.5,0.5,0.5)[]"));
	//lSys[0]->setString(&string("E(0.1,0.1,1)\\(45)/(45)/(45)/(45)R(0.1,0.1,1)\\(45)F(1,1,1)\\(45)E(0.1,0.1,1)/(45)\\(45)/(45)/(45)/(45)\\(45)R(0.1,0.1,1)/(45)F(1,1,1)/(45)E(0.1,0.1,1)\\(45)\\(45)\\(45)/(45)R(0.1,0.1,1)\\(45)F(1,1,1)\\(45)E(0.1,0.1,1)/(45)/(45)\\(45)\\(45)\\(45)"));
	//lSys[0]->setString(&string("!(0.08)\\(45)R(84.8528137423857,84.8528137423857,1)/(45)F(0,120,1)/(45)E(84.8528137423857,84.8528137423857,1)\\(45)"));

	//BASIC TREE
	//r = RuleData("","A(x,W,1)","","!(W)F(0,x,0)[&(a)B(x*R,W*w,0)[^(b)L]]-(d)A(x*r,W*w,0)[^(b)L]"); 
	//rules.insert(rules.end(),r);
	//r = RuleData("","B(x,W,1)","","!(W)F(0,x,0)[\\(A)$C(x*R,W*w,0)[^(b)L]]C(x*r,W*w,0)[^(b)L]");
	//rules.insert(rules.end(),r);
	//r = RuleData("","C(x,W,1)","","!(W)F(0,x,0)[/(A)$B(x*R,W*w,0)[^(b)L]]B(x*r,W*w,0)[^(b)L]");		

	/*
	r = RuleData("A(x,m,I)","!(m)F(O,x,O)[&(a)B(x*R,m*w,O)]-(d)A(x*r,m*w,O)[^(b)L]"); 
	rules.insert(rules.end(),r);
	r = RuleData("B(x,m,I)","!(m)F(O,x,O)[\\(c)$C(x*R,m*w,O)]C(x*r,m*w,O)[^(b)L]");
	rules.insert(rules.end(),r);
	r = RuleData("C(x,m,I)","!(m)F(O,x,O)[/(c)$B(x*R,m*w,O)]B(x*r,m*w,O)[^(b)L]");
	*/
	r = RuleData("A(x,m,I)","!(m)F(O,x,O)[&(a)B(x*R,m*w,O)]-(d)A(x*r,m*w,O)"); 
	rules.insert(rules.end(),r);
	r = RuleData("B(x,m,I)","!(m)F(O,x,O)[\\(c)$C(x*R,m*w,O)]C(x*r,m*w,O)");
	rules.insert(rules.end(),r);
	r = RuleData("C(x,m,I)","!(m)F(O,x,O)[/(c)$B(x*R,m*w,O)]B(x*r,m*w,O)");
	rules.insert(rules.end(),r);
	r = RuleData("F(x,m,I)","D(m,m,I)");
	rules.insert(rules.end(),r);
	r = RuleData("F(x,m,t)","F(t/I*m,m,t)");
	rules.insert(rules.end(),r);
	
	lSys[0]->drawChars = string("FD");
	table['r'] = 0.9;//0.9//0.9//0.9//0.9
	table['R'] = 0.9;//0.6//0.9//0.8//0.7
	table['a'] = 30*DEG2RAD;//45; //45 //45 //45//30
	table['c'] = 30*DEG2RAD;//45; //45 //45 //45//-30
	//table['d'] = 120*DEG2RAD;//137.5;
	table['d'] = 90*DEG2RAD;//137.5;
	table['b'] = 22.5*DEG2RAD;
	table['w'] = 0.707;
	table['O'] = 0;
	table['I'] = 1;
	lSys[0]->setParaData(&table);
	table.clear();
	lSys[0]->setRules(&rules);
	rules.clear();
	//lSys[1]->setString(&string("!(0.03)F(0,1,0)[&(a)B(1*R,0.03*w,0)]-(d)A(1*r,0.03*w,0)[^(b)L]"));
	//lSys[0]->setString(&string("!(0.01)R(10,10,1)"));

	trops[0].first = 0.0f;
	trops[0].second = XMFLOAT3(0,0,0);

	LoadDataTreeSystem tld;
	tld.lsystemPointer = lSys;
	tld.tropisms = trops;
	tld.parserPointer = parser;
	tld.numSystems = numSystems;
	tld.leafShader = shaders[3];

	//It's highly important that with the tree system
	//the shader data is passed in before the load function, as this data is used in the load function
	//where it's passed on to the trees that the system will be using.
	//This is pretty bad
	t->passInShader(shaders[0]);
	t->setShaderTechniqueName("TexturePhongBuffer");
	t->Load(d3dDevice_,d3dContext_,&tld);
	renderObjects[0] = t;

	LitRawVerts* rv = new LitRawVerts;
	VertexPosRV vertices[] =
    {
        { XMFLOAT3( -0.01f,  0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f,  0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f,  0.01f,  0.01f ) },
        { XMFLOAT3( -0.01f,  0.01f,  0.01f ) },

        { XMFLOAT3( -0.01f, -0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f, -0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f, -0.01f,  0.01f ) },
        { XMFLOAT3( -0.01f, -0.01f,  0.01f ) },

        { XMFLOAT3( -0.01f, -0.01f,  0.01f ) },
        { XMFLOAT3( -0.01f, -0.01f, -0.01f ) },
        { XMFLOAT3( -0.01f,  0.01f, -0.01f ) },
        { XMFLOAT3( -0.01f,  0.01f,  0.01f ) },

        { XMFLOAT3(  0.01f, -0.01f,  0.01f ) },
        { XMFLOAT3(  0.01f, -0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f,  0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f,  0.01f,  0.01f ) },

        { XMFLOAT3( -0.01f, -0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f, -0.01f, -0.01f ) },
        { XMFLOAT3(  0.01f,  0.01f, -0.01f ) },
        { XMFLOAT3( -0.01f,  0.01f, -0.01f ) },

        { XMFLOAT3( -0.01f, -0.01f,  0.01f ) },
        { XMFLOAT3(  0.01f, -0.01f,  0.01f ) },
        { XMFLOAT3(  0.01f,  0.01f,  0.01f ) },
        { XMFLOAT3( -0.01f,  0.01f,  0.01f ) }
    };

	WORD indices[] =
    {
        3,   1,  0,  2,  1,  3,
        6,   4,  5,  7,  4,  6,
        11,  9,  8, 10,  9, 11,
        14, 12, 13, 15, 12, 14,
        19, 17, 16, 18, 17, 19,
        22, 20, 21, 23, 20, 22
    };

	rv->Load(d3dDevice_, 
		vertices, sizeof(VertexPosRV), 24,
		indices, sizeof(WORD), 36, 
		NULL, NULL, 0);
	rv->passInShader(shaders[1]);
	//rv->setShaderTechniqueName("FlatColour");
	rv->setPosition(XMFLOAT3(0.0f,0.5f,0.0f));
	rv->setRotation(&XMMatrixRotationRollPitchYaw( 1.57079633f ,0.0f, 0.0f ));// 1.57079633f
	renderObjects[1] = rv;

	Dome* skybox = new Dome;
	skybox->setDome(d3dDevice_,5,5000);
	skybox->passInShader(shaders[2]);
	skybox->setShaderTechniqueName("RenderSkybox");
	renderObjects[2] = skybox;

	HUD* hud = new HUD;
	LoadDataHUD hd;
	const int numOfFonts = 1;
	string* paths [numOfFonts];
	string one = string("fontdata.txt");
	paths[0] = &one;
	hd.filePaths = paths;
	hd.numOfFonts = numOfFonts;
	hd.screenHeight = SCREEN_HEIGHT;//600;
	hd.screenWidth = SCREEN_WIDTH;//800;
	hud->passInShader(shaders[4]);
	hud->setShaderTechniqueName("Text");
	hud->Load(d3dDevice_,&hd);
	renderObjects[3] = hud;

	pair<char,float> cInsert ('c',speed);
	GlobalData::gbMap->erase('c');
	GlobalData::gbMap->insert(cInsert);

	camera_2.SetPosition(XMVectorSet(0.0f,0.0f,-1.0f, 0.0f));

    return true;
}


void ShaderGameV2::UnloadContent( )
{
	//Crashes 
	//for (int i=0;i<numRObjects;i++)
	//	renderObjects[i]->~RenderObject();
	delete[] renderObjects;//[numRObjects];
	delete[] shaders;
}


void ShaderGameV2::Update( float dt )
{
    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
	float forwardBack = 0.0f;
	float leftRight = 0.0f;
	float lightForwardBack = 0.0f;
	float lightLeftRight = 0.0f;

	HRESULT result;
	// Read the keyboard device.
	result = keyboardDevice_->GetDeviceState( sizeof( keyboardKeys_ ), ( LPVOID )&keyboardKeys_ );
	if(FAILED(result))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			keyboardDevice_->Acquire();
		}
		else
		{
		}
	}
	
	// Read the mouse device.
	result = mouseDevice_->GetDeviceState( sizeof ( mouseState_ ), ( LPVOID ) &mouseState_ );
	if(FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			mouseDevice_->Acquire();
		}
		else
		{
		}
	}

    // QUIT
    if( GetAsyncKeyState( VK_ESCAPE ) )
	{
		//swapChain_->SetFullscreenState(FALSE,NULL);
		PostQuitMessage( 0 );
	}

	//Full Screen
	if( KEYDOWN( prevKeyboardKeys_, DIK_F1 ) && !KEYDOWN(keyboardKeys_, DIK_F1 ) )
	{
		BOOL test;
		swapChain_->GetFullscreenState(&test,NULL);
		if (test)
			swapChain_->SetFullscreenState(FALSE,NULL);
		else
			swapChain_->SetFullscreenState(TRUE,NULL);
	}

	//Camera movement
	float camSpeed = speed*dt;
	if( KEYDOWN( prevKeyboardKeys_, DIK_W ) )
	{ 
		forwardBack += camSpeed;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_S ) )
	{ 
		forwardBack -= camSpeed;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_A ) )
	{ 
		leftRight -= camSpeed;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_D ) )
	{ 
		leftRight += camSpeed;
	}

	//TODO
	//VERY BAD new class needed with movment updates within
	//Light Movement
	if( KEYDOWN( prevKeyboardKeys_, DIK_UP ) )
	{ 
		 lightLeftRight += camSpeed;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_DOWN ) )
	{ 
		lightLeftRight -= camSpeed;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_LEFT ) )
	{ 
		lightForwardBack -= camSpeed;
	}

	if( KEYDOWN( prevKeyboardKeys_, DIK_RIGHT ) )
	{ 
		lightForwardBack += camSpeed;
	}
	XMFLOAT3 p = ((LitRawVerts*)renderObjects[1])->getPosition();
	p.x += lightForwardBack;
	p.z += lightLeftRight;
	((LitRawVerts*)renderObjects[1])->setPosition(p);

	//Wire frame change
	if( KEYDOWN( prevKeyboardKeys_, DIK_M ) && !KEYDOWN(keyboardKeys_, DIK_M ) )
	{ 
		D3D11_RASTERIZER_DESC rasterDesc; 
		m_rasterState->GetDesc(&rasterDesc);
		if (rasterDesc.FillMode == D3D11_FILL_SOLID)
			rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
		else
			rasterDesc.FillMode = D3D11_FILL_SOLID;

		// Create the rasterizer state from the description we just filled out.
		d3dDevice_->CreateRasterizerState(&rasterDesc, &m_rasterState);
		// Now set the rasterizer state.
		d3dContext_->RSSetState(m_rasterState);
	}

	if((mouseState_.lX != prevMouseState_.lX) || (mouseState_.lY != prevMouseState_.lY))
	{
		yawDelta += mouseState_.lX * dt;
		pitchDelta += mouseState_.lY * dt;
		mousePosX_ += mouseState_.lX;
		mousePosY_ += mouseState_.lY;
		memcpy( &prevMouseState_, &mouseState_, sizeof( mouseState_ ) );
	}

	if(mouseState_.lZ != prevMouseState_.lZ)
	{
		//if (mouseState_.lZ > prevMouseState_.lZ)
		speed += (mouseState_.lZ/100);
		//else
		//	speed -= 10;
		pair<char,float> cInsert ('c',speed);
		GlobalData::gbMap->erase('c');
		GlobalData::gbMap->insert(cInsert);	
		memcpy( &prevMouseState_, &mouseState_, sizeof( mouseState_ ) );
	}

	BasicInputData d;
	d.keyboardKeys = keyboardKeys_;
	d.prevKeyboardKeys = prevKeyboardKeys_;

	if (renderObjects > 0)
		for (int i =0; i<numRObjects;i++)
			renderObjects[i]->Update(d3dDevice_, d3dContext_, dt, &d);

	memcpy( prevKeyboardKeys_, keyboardKeys_, sizeof( keyboardKeys_ ) );

	camera_2.ApplyRotation( pitchDelta, yawDelta );
	camera_2.ApplyPosition( forwardBack, leftRight );
}


void ShaderGameV2::Render( )
{
    //if( d3dContext_ == 0 )
    //    return;
									  //0.25f
    float clearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
    d3dContext_->ClearRenderTargetView( backBufferTarget_, clearColor );
    d3dContext_->ClearDepthStencilView( depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0 );
	RenderData d;
	d.projMat = &camera_2.GetProjMatrix();
	d.viewMat = &camera_2.GetViewMatrix();
	d.camPos = &camera_2.GetPosition();
	for (int i =0; i<numRObjects;i++){
		renderObjects[i]->Render(d3dContext_, &d);
		d3dContext_->OMSetBlendState(0, 0, 0xffffffff);
		d3dContext_->RSSetState(m_rasterState);
	}

	
	//g_pVsgDbg->CaptureCurrentFrame();
    swapChain_->Present( testingSwap, 0 );
	//if (testingSwap == 1)
	//	testingSwap = 0;
	//else
	//	testingSwap = 1;
}