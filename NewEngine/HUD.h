/*
	Font Class based upon: http://www.rastertek.com/dx11tut13.html
*/
#ifndef _HUD_H_
#define _HUD_H_

#include"Sentence.h"
#include<string>

struct LoadDataHUD
{
	string** filePaths;
	int numOfFonts;
	int screenHeight;
	int screenWidth;
};

class HUD : public RawVerts
{
    public:
		HUD();
        virtual ~HUD();
		void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data);

		//It's highly important that with the tree system
		//the shader data is passed in before the load function, as this data is used in the load function
		//where it's passed on to the trees that the system will be using.
		//This is pretty bad
		void Load(ID3D11Device* d3dDevice, LoadDataHUD*);

		void Render(ID3D11DeviceContext* dc, void* d);
    private:
		Sentence* sentence1;
		ID3D11DepthStencilState* m_depthStencilState;
		ID3D11DepthStencilState* m_depthDisabledStencilState;
		ID3D11BlendState* m_alphaEnableBlendingState;
	    ID3D11BlendState* m_alphaDisableBlendingState;
		float time;
		int frames;
};

#endif