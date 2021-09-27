/*
    Render object - Matthew Caldwell
*/

#ifndef _GRASS_H_
#define _GRASS_H_

#include"GrassBlade.h"

class Grass : public GrassBlade
{

    public:
        Grass( );
		virtual ~Grass( );
        virtual void Render(ID3D11DeviceContext*, void*);
		virtual void Update(float);
		VertexPosGB * GeneratePatch( unsigned int bladeCnt, float scale );
		void Generate(ID3D11Device* d3dDevice, XMFLOAT3 surfaceCenter, float surfaceR, unsigned int patchBladeCnt, float inPatchRadius );
	private:
		unsigned long m_lastTime;
		int	m_numGrassBB;
		float patchRadius;
		int	patchCountX;
		int	patchCountZ;
		unsigned int patchCount;
		//XMFLOAT2* patchPos;
};
#endif