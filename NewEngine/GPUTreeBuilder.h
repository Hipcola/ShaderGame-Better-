/*
    Tree building compute shader - Matthew Caldwell
*/

#ifndef _TREECOMPUTE_H_
#define _TREECOMPUTE_H_

#include"ComputeShader.h"
#include<windows.h>
#include<DirectxMath.h>
#include"RawVerts.h"

class GPUTreeBuilder : public ComputeShader
{
    public:
        GPUTreeBuilder( );
		virtual ~GPUTreeBuilder( );
		void Load(ID3D11Device* d3dDevice, unsigned int sides);
		void buildTree(ID3D11DeviceContext* d3dContext, 
			ID3D11ShaderResourceView* versSRV, 
			ID3D11ShaderResourceView* indsSRV, 
			ID3D11UnorderedAccessView* resultVersUAV, 
			ID3D11UnorderedAccessView* resultIndsUAV, 
			UINT vertSize, UINT indSize, ID3D11Device* d3dDevice);
	protected:
		ID3D11Buffer* inputSize;
		ID3D11Buffer* texCoords;
		ID3D11ShaderResourceView* texCoordsSRV;
		unsigned int inSides;
};

#endif