/*
    Render object - Matthew Caldwell
*/

#ifndef _LITVERTS_H_
#define _LITVERTS_H_

#include"RawVerts.h"

class LitRawVerts : public RawVerts
{
    public:
        LitRawVerts( );
		virtual ~LitRawVerts( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float, void*);
		//XMFLOAT4 getLightColour();
		//XMFLOAT3 getLightPosition();
		//void setLightColour();
		//void setLightPosition();
		XMFLOAT4 lightColour;
		XMFLOAT3 lightPosition;
	private:

};
#endif