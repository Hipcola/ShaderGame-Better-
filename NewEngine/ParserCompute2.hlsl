//--------------------------------------------------------------------------------------
// File: ParserCompute2.hlsl
// 
// Scan. To be 
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
	uint depthTotal;
	uint buff2;
	uint buff3;
}

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

RWStructuredBuffer<uint7>		sums	 : register(u1);

//groupshared uint4 gSum[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint size;
	uint blank;
	sums.GetDimensions(size,blank);
	//size = (size/GROUPDIMX)+2;

	uint counter  = 0;
	uint counter2 = 0;
	uint counter3 = 0;
	uint counter4 = 0;

	[loop]
	for(uint i = 0;i<size;i++)
	{
		uint7 s = sums[i];
		sums[i].x = counter;
		sums[i].y = counter2;
		sums[i].z = counter3;
		sums[i].w = counter4;
		counter  += s.x;
		if ((s.y + counter4) > (counter2 + counter4))
		{
			counter2 = s.y+counter4;
			//counter4 = s.w+counter4;
		}
		//else{
			counter4 += s.w;
		//}
		counter3 += s.z;
	}

	/*
	uint groupOffset = (Gid.x*GROUPDIMX);
	uint ai = GTid.x+groupOffset;
	uint bi = ai + THREADS;
	uint bankOffsetA = CONFLICT_FREE_OFFSET(ai);
	uint bankOffsetB = CONFLICT_FREE_OFFSET(bi);
	uint localA = ai + bankOffsetA;
	uint localB = bi + bankOffsetB;

	if (ai < size)
	{
		uint7 s = sums[ai];
		gSum[localA] = uint4(s.x, s.y, s.z, s.w);
	}
	else
		gSum[localA] = 0;

	if (bi < size)
	{
		uint7 s = sums[bi];
		gSum[localB] = uint4(s.x, s.y, s.z, s.w);
	}
	else
		gSum[localB] = 0;

	uint offset = 1;
	//reduction step
	[loop]
	for (int a = GROUPDIMX>>1; a > 0; a >>= 1) // build sum in place up the tree 
	{
		//syn
		GroupMemoryBarrierWithGroupSync();
		if (GTid.x < a) 
		{
			int ai2 = offset*(2*GTid.x+1)-1;
			int bi2 = offset*(2*GTid.x+2)-1;
 
			ai2 += CONFLICT_FREE_OFFSET(ai2); 
			bi2 += CONFLICT_FREE_OFFSET(bi2); 

			gSum[bi2] += gSum[ai2];
		}
		offset *= 2;
	}

	// clear the last element 
	//*
	if (GTid.x == 0) 
	{
		//newSumVals[Gid.x].x = rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];
		gSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)] = uint4(0,0,0,0);
	}

	//offset >>= 1; 
	[loop]
	for (int b = 1; b < GROUPDIMX; b *= 2) // traverse down tree & build scan 
	{ 
		offset >>= 1; 
		//syn
		GroupMemoryBarrierWithGroupSync();
		if (GTid.x < b)
		{ 	
			int aii = offset*(2*GTid.x+1)-1;
			int bii = offset*(2*GTid.x+2)-1;
			aii += CONFLICT_FREE_OFFSET(aii); 
			bii += CONFLICT_FREE_OFFSET(bii); 

			uint4 gs = gSum[aii];
			gSum[aii] = gSum[bii];
			gSum[bii] += gs;

			/* Inclusive scan
			int aii = (offset*2)*(2*GTid.x+1)-1;
			int sii = aii+offset;
			sii += CONFLICT_FREE_OFFSET(sii);
			aii += CONFLICT_FREE_OFFSET(aii);
			gSum[sii] += gSum[aii];/
		}
	}
	GroupMemoryBarrierWithGroupSync();

	if (ai < size)
	{
		uint4 l = gSum[localA];
		uint7 s = set7Int(l.x,l.y,l.z,l.w,0,0,0);
		sums[ai] = s;
	}

	if (bi < size)
	{
		uint4 l = gSum[localB];
		uint7 s = set7Int(l.x,l.y,l.z,l.w,0,0,0);
		sums[bi] = s;
	}
	//*/
}