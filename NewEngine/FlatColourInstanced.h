/*
    Flat colour shader - Matthew Caldwell
*/

#ifndef _FLATCOLOURI_H_
#define _FLATCOLOURI_H_

#include"Shader.h"
#include<windows.h>
#include<xnamath.h>

struct ShaderDataFCI
{
	XMMATRIX* worldMat;
	XMMATRIX* viewMat;
	XMMATRIX* projMat;
};

class FlatColourInstanced : public Shader
{
    public:
        FlatColourInstanced( );
		virtual ~FlatColourInstanced( );
        void Load(ID3D11Device* d3dDevice);
		void preRender(ID3D11DeviceContext*, void*);
		ID3D11InputLayout* inputLayout_;
};


#endif