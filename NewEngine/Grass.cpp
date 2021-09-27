#include"Grass.h"
#include"GrassShader.h"
Grass::Grass()
{
}

Grass::~Grass( )
{
}

/*void Grass::Load(ID3D11Device* d3dDevice)
{
	RawVertsInstanced::Load(d3dDevice);
}*/
void Grass::Update(float delta)
{

}

//  geometry for single grass blade 
static VertexPosGB grassBBVerts[] =
{
	//					x     y      z				  
	{  XMFLOAT3( -10.0f,   0.0f,   0.0f) },
	{  XMFLOAT3(  10.0f,   0.0f,   0.0f) },
	{  XMFLOAT3(  10.0f,  -10.0f,  0.0f) },

	{  XMFLOAT3(  -10.0f,   0.0f,  0.0f) },
	{  XMFLOAT3(   10.0f,  -10.0f, 0.0f) },
	{  XMFLOAT3(  -10.0f,  -10.0f, 0.0f) }
};

//  x,z instance positions ...
static XMFLOAT2 defaultPatchPos[] =
{
	//			   x      z					
	XMFLOAT2(  20.0f, 0.0f ) ,
	XMFLOAT2(  0.0f,  0.0f ) ,
	XMFLOAT2( -20.0f, 0.0f ) ,

	XMFLOAT2(  20.0f, -20.0f ) ,
	XMFLOAT2(  0.0f,  -20.0f  ) ,
	XMFLOAT2( -20.0f, -20.0f ) ,

	XMFLOAT2(  20.0f,  40.0f ) ,
	XMFLOAT2(  20.0f,  40.0f ) ,
	XMFLOAT2(  0.0f,   40.0f ) ,
	XMFLOAT2( -20.0f,  40.0f ),
	XMFLOAT2( -40.0,   40.0f ),

	XMFLOAT2(  40.0f,  20.0f ) ,
	XMFLOAT2(  20.0f,  20.0f ) ,
	XMFLOAT2(  0.0f,   20.0f ) ,
	XMFLOAT2( -20.0f,  20.0f ) ,
	XMFLOAT2( -40.0f,  20.0f ) ,
};

VertexPosGB* Grass::GeneratePatch( unsigned int bladeCnt, float scale )
{
	m_numGrassBB = bladeCnt;
	VertexPosGB * m_grassVerts = new VertexPosGB[m_numGrassBB*6];

	XMMATRIX mat = XMMatrixIdentity();
	XMMATRIX matRandom;

	float pi = 3.141592654f;
	for( int j=0; j< m_numGrassBB; j++ )
	{
		// Generate each quad at random position, orientation, height
		matRandom = XMMatrixIdentity();
	//	float scale = 10.0f;
		float angle = ((float)rand()/RAND_MAX - 0.5f)*2 * pi; // angle = [-pi,pi]
		float dx = ((float)rand()/RAND_MAX - 0.5f)*2 * scale;
		float dz = ((float)rand()/RAND_MAX - 0.5f)*2 * scale;
		float heightScale = ((float)rand()/RAND_MAX - 0.5f) / 2.0f + 1.0f;

		mat = XMMatrixRotationY(angle);
		matRandom = XMMatrixMultiply( matRandom, mat );
		mat = XMMatrixTranslation( dx, 0.0f, dz);			
		matRandom = XMMatrixMultiply( matRandom, mat );
		mat = XMMatrixScaling( 1.0f, 1.0f, heightScale);			
		matRandom = XMMatrixMultiply( matRandom, mat );

		// Apply the transformation to each vertex of the quad
		for( DWORD i=0; i< 6; i++ )
		{
			XMVECTOR pos, outPos;
			pos = XMVECTOR();
			outPos = XMVECTOR();
			XMVectorSetX(pos, grassBBVerts[i].pos.x);
			XMVectorSetY(pos, grassBBVerts[i].pos.y);
			XMVectorSetZ(pos, grassBBVerts[i].pos.z);
			XMVectorSetW(pos, 1.0);
			outPos = XMVector3Transform(pos, matRandom);
			int index = j * 6 + i;
			m_grassVerts[index].pos.x = XMVectorGetX(outPos);
			m_grassVerts[index].pos.y = XMVectorGetY(outPos);
			m_grassVerts[index].pos.z = XMVectorGetZ(outPos);
			m_grassVerts[index].tex0 = grassBBVerts[i].tex0;
		}
	}
	return m_grassVerts;
}

void Grass::Generate(ID3D11Device* d3dDevice, XMFLOAT3 surfaceCenter, float surfaceR, unsigned int patchBladeCnt, float inPatchRadius )
{
	patchRadius     = inPatchRadius;
	VertexPosGB * verts = GeneratePatch( patchBladeCnt, patchRadius );

	patchCountX		= int( surfaceR/patchRadius );
	patchCountZ		= int( surfaceR/patchRadius );
	patchCount		= (2*patchCountX-1)*(2*patchCountZ-1);
	InstanceDataG* patchPos	= new InstanceDataG[patchCount];
	//visiblePatches  = new XMFLOAT2[patchCount];
	int k = 0;
	for(int i = -(patchCountX-1); i < patchCountX; i++)
	{
		for(int j = -(patchCountZ-1); j < patchCountZ; j++)
		{
			patchPos[k].pos.x = surfaceCenter.x + i*patchRadius;
			patchPos[k].pos.y = surfaceCenter.z + j*patchRadius;
			k++;
		}

	}
	
	LoadDataG l;
	l.verts = grassBBVerts;
	l.insts = patchPos; 
	l.numOfVerts = 6;//m_numGrassBB*6;
	l.numOfInsts = patchCount;

	GrassBlade::Load(d3dDevice,&l);
}

void Grass::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	//XMMATRIX* viewMat = &((renderData*)data)->camera->GetViewMatrix();
    //viewMat = &XMMatrixTranspose(*viewMat );

	ShaderDataGS d;
	d.viewMat = ((renderData*)data)->viewMat;
	d.projMat = ((renderData*)data)->projMat;
	d.worldMat = &XMLoadFloat4x4(&worldMat);
	float f6 = (float)(timeGetTime() - m_lastTime);
	f6 = f6/1000;
	d.time = &f6;
	((GrassShader*)&shaders[0])->preRender(d3dContext,&d);
	
	unsigned int stride[2];
	unsigned int offset[2] = {0,0};
	stride[0] = sizeof(VertexPosRVI);
	stride[1] = sizeof(InstanceDataG);

	ID3D11Buffer* bufferPointers[2];
	bufferPointers[0] = vertexBuffer_;	
	bufferPointers[1] = instanceBuffer_;
    d3dContext->IASetVertexBuffers( 0, 2, bufferPointers, stride, offset );
    d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    ID3DX11EffectTechnique* multiTexTechnique;
    multiTexTechnique = shaders[0].effect->GetTechniqueByName( effectName );

    D3DX11_TECHNIQUE_DESC techDesc;
    multiTexTechnique->GetDesc( &techDesc );

    for( unsigned int p = 0; p < techDesc.Passes; p++ )
    {
        ID3DX11EffectPass* pass = multiTexTechnique->GetPassByIndex( p );

        if( pass != 0 )
        {
            pass->Apply( 0, d3dContext );
			d3dContext->DrawInstanced( vertexCount,    	// number of vertices per instance
					  instanceCount,	 // number of instances
					  0,		 // Index of the first vertex
					  0		 // Index of the first instance
			);
        }
    }
}