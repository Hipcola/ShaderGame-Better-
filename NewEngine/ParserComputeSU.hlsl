//--------------------------------------------------------------------------------------
// File: ParserCompute5.1.hlsl
//
// L-System parser
//--------------------------------------------------------------------------------------

#define NUM_BANKS 16 
#define GROUPDIMX 32//256
#define THREADS GROUPDIMX/2
#define PARSECHUNK 8
#define LOG_NUM_BANKS 4 
#define CONFLICT_FREE_OFFSET(n) ((n) >> LOG_NUM_BANKS) 

const float DEG2RAD = 3.14159f / 180;

cbuffer buffSize
{
	uint inputBufferSize;
	uint DSize;
	uint numBr;
	uint currDepth;
}

struct int6
{
	int x;
	int y;
	int z;
	int w;
	int u;
	int v;
};

struct uint7
{
	uint x;
	uint y;
	uint z;
	uint w;
	uint u;
	uint v;
	uint q;
};

RWStructuredBuffer<uint7>	sums			: register(u1);
RWByteAddressBuffer			depthSizes		: register(u5);
RWStructuredBuffer<uint2>	test			: register(u7);

int6 se6Int(int a, int b, int c, int d, int e, int f)
{
	int6 output;
	output.x = a;
	output.y = b;
	output.z = c;
	output.w = d;
	output.u = e;
	output.v = f;
	return output;
}

uint7 set7Int(uint a, uint b, int c, uint d, uint e, uint f, uint g)
{
	uint7 output;
	output.x = a;
	output.y = b;
	output.z = c;
	output.w = d;
	output.u = e;
	output.v = f;
	output.q = g;
	return output;
}

[numthreads(1, 1, 1)]
void CSMain()
{
	uint counter = 0;
	uint counter2 = 0;
	uint depthCheck = 0;

	[loop]
	for(uint i = 0;i<(numBr/2)+1;i++)
	{
		if (sums[i].x > depthCheck)
		{
			uint3 store = uint3(i,counter,counter2);
			if(sums[i].x == 1)
			{
				depthSizes.Store3(0,uint3((counter2/(GROUPDIMX*PARSECHUNK))+1,1,1));
				depthSizes.Store3(12,uint3(0,0,0));
			}
			//if(sums[i].x == DSize)
			//	store.x = (numBr/2)+1;
			//uint3 read = depthSizes.Load3((depthCheck+1)*12);
			//read.x = i;
			depthSizes.Store3((depthCheck+2)*12, store);
			//depthSizes.Store3((depthCheck+1)*12, read);
			depthCheck++;
		}
		//uint swapper = counter;
		sums[i].v = counter;
		//sums[i].w = counter2;
		counter += sums[i].q;
		counter2 += sums[i].u;
	}
	test[0] = uint2(counter,counter2);
}