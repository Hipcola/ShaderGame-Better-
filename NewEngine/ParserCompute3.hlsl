//--------------------------------------------------------------------------------------
// File: ParserCompute2.hlsl
// 
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

struct int6
{
	int x;
	int y;
	int z;
	int w;
	int u;
	int v;
};

StructuredBuffer<uint>		drawChars		: register(t0);
StructuredBuffer<uint3>		inLData			: register(t1);

RWStructuredBuffer<uint7>	sums			: register(u1);
RWStructuredBuffer<int4>	countVals		: register(u2);
RWStructuredBuffer<uint>	dSums			: register(u3);
RWByteAddressBuffer/*<uint3>*/	depthSizes	: register(u5);
RWStructuredBuffer<uint7>	brackets		: register(u6);
RWStructuredBuffer<uint>	depthCounters	: register(u7);
//RWStructuredBuffer<uint4>   newSumVals	: register(u3);
groupshared uint prefixSum[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];

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

void depthBracketCount(uint depthPointer, uint gOffset, uint startPos, int4 counts)
{
	uint dcSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);

	int vers = counts.x;
	int depthPos = counts.w;
	//int brC = counts.z;
	[loop]
	for (uint i = 0; i<PARSECHUNK; i++)
	{
		int parsePos = startPos+i;
		[branch]
		if (parsePos < inputBufferSize)
		{
			uint3 input = inLData[parsePos].x;
			[branch]
			switch(input.x)
			{
				case 91: // [
					depthCounters[depthPointer+(depthPos*GROUPDIMX) + gOffset]++; 
					depthPos++;
					//paramData[input.y] = asfloat(brC);
					//brackets[brC] = set7Int(depthPos, parsePos, 0, 0, 0, vers, 0);
					//brC++;
					break;
				case 93: // ]
					depthPos--;
					depthCounters[depthPointer+(depthPos*GROUPDIMX) + gOffset]++;
					//paramData[input.y] = asfloat(brC);
					//brackets[brC] = set7Int(depthPos, parsePos, 0, 0, 0, vers, 0);
					//brC++;
					break;
				default:
					/*
					uint ptr;
					ptr = dcSize;
					ptr >>= 1; 
					bool drawCheck = false;
					[allow_uav_condition]
					for (int max = dcSize; max > 0; max >>=1)
					{
						uint lookup = drawChars[ptr];
						if (input.x == lookup)
						{
							drawCheck = true;
							break;
						}
						else if (input.x > lookup && (ptr*2) < dcSize)
							ptr *=  2;
						else if (input.x < lookup && (ptr/2) >= 0)
							ptr >>= 1;
					}
					if (drawCheck)
					{
						vers++;
					}
					//*/
					break;
					;

			}
		}else{
			i = PARSECHUNK;
		}
	}
}

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint groupOffset = Gid.x*GROUPDIMX;
	uint ai = GTid.x+groupOffset;
	uint bi = ai + THREADS;
	uint parseStartA = ai*PARSECHUNK;
	uint parseStartB = bi*PARSECHUNK;
	uint bankOffsetA = CONFLICT_FREE_OFFSET(GTid.x); 
	uint bankOffsetB = CONFLICT_FREE_OFFSET(GTid.x + THREADS);
	uint localA = GTid.x + bankOffsetA;
	uint localB = (GTid.x + THREADS) + bankOffsetB;

	uint gOffset = Gid.x*(depthTotal*GROUPDIMX);
	uint gOG = Gid.x * depthTotal;
	/*uint size;
	uint blank;
	countVals.GetDimensions(size,blank);
	size = (size/GROUPDIMX)+2;*/

	[loop]
	for(uint i=0; i<depthTotal;i++)
	{
		depthCounters[GTid.x+(i*GROUPDIMX) + gOffset] = 0;
		depthCounters[(GTid.x+THREADS)+(i*GROUPDIMX) + gOffset] = 0;
	}

	if (Gid.x > 0)
	{
		countVals[ai].x += sums[Gid.x].x;
		countVals[ai].w += sums[Gid.x].w;
		countVals[bi].x += sums[Gid.x].x;
		countVals[bi].w += sums[Gid.x].w;
	}
	GroupMemoryBarrierWithGroupSync();

	if (parseStartA < inputBufferSize)
	{
		int4 counts = countVals[ai];
		depthBracketCount(GTid.x, gOffset, parseStartA, counts);
	}

	if (parseStartB < inputBufferSize)
	{
		int4 counts = countVals[bi];
		depthBracketCount(GTid.x+THREADS, gOffset, parseStartB, counts);
	}

	//uint depthBucketCounter = 0;

	[loop]
	for(uint i=0; i<depthTotal;i++)
	{
		prefixSum[localA] = depthCounters[GTid.x+(i*GROUPDIMX) + gOffset];
		prefixSum[localB] = depthCounters[(GTid.x+THREADS)+(i*GROUPDIMX) + gOffset];
		//Finding pointers to rewrite
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

				prefixSum[bi2] += prefixSum[ai2];
			}
			offset *= 2;
		}

		// clear the last element 
		if (GTid.x == 0) 
		{
			//newSumVals[Gid.x].x = rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];

			uint end = prefixSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];
			dSums[gOG+i] = end;
			//depthBucketCounter += end;
			//uint3 test = uint3(depthBucketCounter,0,0);
			//depthSizes.Store3((i*4)*3 /*i*12*/, test);
			//depthBucketCounter += end;
			prefixSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)] = 0;
		}

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

				uint l = prefixSum[aii];
				prefixSum[aii] = prefixSum[bii];
				prefixSum[bii] += l;
			}
		}
		GroupMemoryBarrierWithGroupSync();
		depthCounters[GTid.x+(i*GROUPDIMX) + gOffset] = prefixSum[localA];
		depthCounters[(GTid.x+THREADS)+(i*GROUPDIMX) + gOffset] = prefixSum[localB];
	}

	/*
	if (GTid.x == 0) 
	{
		uint3 test = uint3(depthBucketCounter,0,0);
		depthSizes.Store3((i*4)*3, test);
	}
	*/

	/*
	GroupMemoryBarrierWithGroupSync();
	if (parseStartA < inputBufferSize)
		 countVals[ai].x = prefixSum[localA];

	if (parseStartB < inputBufferSize)
		 countVals[bi].x = prefixSum[localB];
	*/

	//*/
}