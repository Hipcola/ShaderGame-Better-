/*
    Render object - Matthew Caldwell
*/

#ifndef _ROBJECT_H_
#define _ROBJECT_H_
#include"Shader.h"
#include<vector>
//#include<Windows.h>
//#include<xnamath.h>
#include<DirectXMath.h>
using namespace DirectX;

struct RenderData
{
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
	XMFLOAT4* camPos;
};

//TODO
//This is even worse than the render data input
//Input is arguably more ambigous than what's needed for rendering
//Expected array size is 256
struct BasicInputData
{
	char* keyboardKeys;
    char* prevKeyboardKeys;
};

class RenderObject
{
    public:
        RenderObject( );
		virtual ~RenderObject( ) = 0;
        virtual void Render(ID3D11DeviceContext*, void*) = 0;
		virtual void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data) = 0;
		//virtual void Load(ID3D11Device*) = 0;
		void setShaderTechniqueName(char*);
		void passInShader(Shader*);
	protected:
		bool loaded;
		Shader* shaders;
		char* effectName;
		int numEffectNames;
		int numShaders;
};

#endif