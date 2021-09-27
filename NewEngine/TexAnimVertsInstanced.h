/*
    Render object - Matthew Caldwell
*/


#ifndef _TEXANIMINVERTS_H_
#define _TEXANIMINVERTS_H_

#include"RawVertsInstanced.h"
#include<ctime>

class TexAnimVertsInstanced : public RawVertsInstanced
{
    public:
        TexAnimVertsInstanced( );
		virtual ~TexAnimVertsInstanced( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Update(float);
		int animCounter;
		time_t lastTime;
		//virtual void Load(ID3D11Device* d3dDevice);
};
#endif