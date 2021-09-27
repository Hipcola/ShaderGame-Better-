/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    ShaderGameV2 - Demonstrates mutiple textures on one object.
*/


#ifndef _TREE_DEMO_H_
#define _TREE_DEMO_H_

#include"Dx11Base.h"
#include"FPSCamera.h"

#include"Dome.h"
#include"SkyShader.h"

#include"Leaves.h"
#include"Texture.h"

#include"TreeSystem.h"
#include"TexturePhongShader.h"

#include"ColourShader.h"
#include"LitRawVerts.h"

//#include"ComputeShader.h"
//#include"LogicalOpParser.h"

#include"Text.h"
#include"HUD.h"


class ShaderGameV2 : public Dx11Base
{
    public:
        ShaderGameV2( );
        virtual ~ShaderGameV2( );

        bool LoadContent( );
        void UnloadContent( );

        void Update( float dt );
        void Render( );

    private:
		ID3D11Buffer* viewCB_;
        ID3D11Buffer* projCB_;
        ID3D11Buffer* worldCB_;
		ID3D11Buffer* buffer;
        XMMATRIX projMatrix_;
		//Camera camera_;
		FPSCamera camera_2;

		//const int?
		int numRObjects;
		int numShaders;
		float speed;
		RenderObject** renderObjects;
		Shader** shaders;
		//LSParser* parser;
		unsigned int testingSwap;
};

#endif