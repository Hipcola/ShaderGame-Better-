#include"HUD.h"
#include"utility.h"
#include"GlobalData.h"
#include<dinput.h>

#define KEYDOWN( name, key ) ( name[key] & 0x80 )
#define BUTTONDOWN(device, key) ( device.rgbButtons[key] & 0x80 )

HUD::HUD()
{
	time = 0.0f;
	frames = 0;
};

HUD::~HUD( )
{
}

void HUD::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data)
{
	BasicInputData* d = (BasicInputData*)data;
	time += delta;
	frames++;
	if (time>1.0f)
	{
		string a = "";
		if(GlobalData::gbMap->at('g') == 1)
		{
			a.append("GPU - ");
		}
		else
		{
			a.append("CPU - ");
		}
		a.append(" FPS:");
		a.append(Utility::convertInt(frames));
		a.append(" CamSpeed:");
		try{
			a.append(Utility::convertFloat(GlobalData::gbMap->at('c')));
		}catch (const std::out_of_range& oor) 
		{
			a.append("0");
		}

		a.append(" Verts:");
		try{
			a.append(Utility::convertInt(GlobalData::gbMap->at('v')));
		}catch (const std::out_of_range& oor) 
		{
			a.append("0");
		}
		a.append(" String:");
		try{
			a.append(Utility::convertInt(GlobalData::gbMap->at('s')));
		}catch (const std::out_of_range& oor) 
		{
			a.append("0");
		}
		a.append(" Time:");
		try{
			a.append(Utility::convertFloat(GlobalData::gbMap->at('t')));
		}catch (const std::out_of_range& oor) 
		{
			a.append("0");
		}
		a.append(" NumBuffs:");
		try{
			a.append(Utility::convertFloat(GlobalData::gbMap->at('b')));
		}catch (const std::out_of_range& oor) 
		{
			a.append("0");
		}
		time = 0.0f;
		frames = 0;
		sentence1->UpdateSentence(&a,10,10,1.f,1.f,1.f);
	}
}

void HUD::Load(ID3D11Device* d3dDevice, LoadDataHUD* data)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	D3D11_BLEND_DESC blendStateDescription;

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create the depth stencil state.
	d3dDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));
	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create the state using the device.
	d3dDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);

	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
    blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	d3dDevice->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);

	// Modify the description to create an alpha disabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	// Create the blend state using the description.
	d3dDevice->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);

	sentence1 = new Sentence();
	sentence1->passInShader(shaders);
	sentence1->setShaderTechniqueName(effectName);
	sentence1->LoadFontData(data->filePaths[0]->c_str());
	sentence1->InitializeSentence(d3dDevice,70,data->screenHeight,data->screenWidth);
	sentence1->UpdateSentence(&string("FPS:"),10,10,1.f,1.f,1.f);
}

void HUD::Render(ID3D11DeviceContext* d3dContext, void* data)
{
//	sentence1->UpdateSentence("F",30,30,1.f,0.f,0.f);
	float blendFactor[4];
	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	// Turn on the alpha blending.
	d3dContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor, 0xffffffff);

	d3dContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	sentence1->Render(d3dContext, data);
	d3dContext->OMSetDepthStencilState(m_depthStencilState, 1);

	d3dContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);
}
