/*
    Render object - Matthew Caldwell
*/

#ifndef _DOME_H_
#define _DOME_H_

#include"RawVerts.h"

class Dome : public RawVerts
{
    public:
        Dome( );
		virtual ~Dome( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta ,void* data);
};
#endif