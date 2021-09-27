/*
    L builder compute shader - Matthew Caldwell
*/

#ifndef _LSYSCOMPUTE_H_
#define _LSYSCOMPUTE_H_

#include"ComputeShader.h"
#include"RawVerts.h"

#define BLOCKSIZE 32
#define PARSECHUNK 8

struct sevenUint{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
	unsigned int e;
	unsigned int f;
	unsigned int g;
	sevenUint() : a(0),b(0),c(0),d(0),e(0),f(0),g(0) {}
	sevenUint(unsigned int aa, unsigned int bb, unsigned int cc, unsigned int dd, unsigned int ee, unsigned int ff, unsigned int gg) : a(aa),b(bb),c(cc),d(dd),e(ee),f(ff),g(gg) {}
};

struct sixUint{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
	unsigned int e;
	unsigned int f;
	sixUint() : a(0),b(0),c(0),d(0),e(0),f(0) {}
	sixUint(unsigned int aa, unsigned int bb, unsigned int cc, unsigned int dd, unsigned int ee, unsigned int ff) : a(aa),b(bb),c(cc),d(dd),e(ee),f(ff) {}
};

struct fourUint{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
	fourUint() : a(0),b(0),c(0),d(0) {}
	fourUint(unsigned int aa, unsigned int bb, unsigned int cc, unsigned int dd) : a(aa),b(bb),c(cc),d(dd) {}
};

struct threeUint{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	threeUint() : a(0),b(0),c(0) {}
	threeUint(unsigned int aa, unsigned int bb, unsigned int cc) : a(aa),b(bb),c(cc) {}
};

struct twoUint{
	unsigned int a;
	unsigned int b;
	twoUint() : a(0),b(0) {}
	twoUint(unsigned int aa, unsigned int bb ) : a(aa),b(bb) {}
};

struct sixint{
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
	sixint() : a(0),b(0),c(0),d(0),e(0),f(0) {}
	sixint(int aa, int bb, int cc, int dd, int ee, int ff) : a(aa),b(bb),c(cc),d(dd),e(ee),f(ff) {}
};

struct fiveint{
	int a;
	int b;
	int c;
	int d;
	int e;
	fiveint() : a(0),b(0),c(0),d(0),e(0) {}
	fiveint(int aa, int bb, int cc, int dd, int ee) : a(aa),b(bb),c(cc),d(dd),e(ee) {}
};

struct fourint{
	int a;
	int b;
	int c;
	int d;
	fourint() : a(0),b(0),c(0),d(0) {}
	fourint(int aa, int bb, int cc, int dd) : a(aa),b(bb),c(cc),d(dd) {}
};

struct threeint{
	int a;
	int b;
	int c;
	threeint() : a(0),b(0),c(0) {}
	threeint(int aa, int bb, int cc) : a(aa),b(bb),c(cc) {}
};


struct twoint{
	int a;
	int b;
	twoint() : a(0),b(0) {}
	twoint(int aa, int bb) : a(aa),b(bb){}
};

class LSystemGPU : public ComputeShader
{
    public:
		int store;
        LSystemGPU( );
		virtual ~LSystemGPU( );
		void Load(ID3D11Device* d3dDevice);
		void run(ID3D11DeviceContext* d3dContext, 
			ID3D11ShaderResourceView** inSRV,  
			ID3D11UnorderedAccessView** resultUAV, UINT numIn, UINT numRes, UINT tableSize, float time);
		/*ID3D11ShaderResourceView***/void parse(ID3D11DeviceContext* d3dContext, ID3D11Device* d3dDevice,
			ID3D11ShaderResourceView** inSRV,  
			ID3D11Buffer** vers, ID3D11Buffer** inds, ID3D11ShaderResourceView** versSRV, ID3D11ShaderResourceView** indsSRV, ID3D11UnorderedAccessView** params, UINT numIn, UINT& versOutput);
	protected:
		ID3D11ComputeShader* CShader2;
		ID3D11ComputeShader* ParseComp1;
		ID3D11ComputeShader* ParseComp2;
		ID3D11ComputeShader* ParseComp3;
		ID3D11ComputeShader* ParseComp4;
		ID3D11ComputeShader* ParseComp5;
		ID3D11ComputeShader* ParseComp6;
		ID3D11ComputeShader* ParseCompBD;
		ID3D11ComputeShader* ParseCompBDG;
		ID3D11ComputeShader* ParseCompNBD;
		ID3D11ComputeShader* ParseCompSU;
		ID3D11ComputeShader* ParseCompS1;
		ID3D11ComputeShader* ParseCompS2;
		//ID3D11ComputeShader* VertConstruct;
		//static const unsigned int groupSizeX = 128;
		ID3D11Buffer* inputSize;
		ID3D11Buffer* inputParser;
		float testing123;
};

#endif