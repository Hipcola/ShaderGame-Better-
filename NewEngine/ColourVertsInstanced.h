/*
    Render object - Matthew Caldwell
*/


#ifndef _COLANIMINVERTS_H_
#define _COLANIMINVERTS_H_

#include"RawVertsInstanced.h"

class ColourVertsInstanced : public RawVertsInstanced
{
    public:
        ColourVertsInstanced( );
		virtual ~ColourVertsInstanced( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Update(float);
};
#endif