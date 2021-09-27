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

StructuredBuffer<uint>		drawChars		: register(t0);
StructuredBuffer<uint3>		inLData			: register(t1);
//StructuredBuffer<float>		paramData		: register(t2);

//RWStructuredBuffer<int4>	sums			: register(u2);
RWStructuredBuffer<int4>	countVals		: register(u2);
RWStructuredBuffer<uint>	dSums			: register(u3);
RWByteAddressBuffer			depthSizes		: register(u5);
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

void depthBracketCount(uint depthPointer, uint groupPointer, uint startPos, int4 counts)
{
	uint dcSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);

	//uint gdSize;
	//drawChars.GetDimensions(gdSize,blank);
	//gdSize /= depthTotal;

	int vers = counts.x;
	int depthPos = counts.w;
	int brC = counts.z;

	//bool dtrue = false;
	//if (depthTotal > 1)
	//	dtrue = true;
	uint gOffset = groupPointer*(depthTotal*GROUPDIMX);

	[loop]
	for (uint i = 0; i<PARSECHUNK; i++)
	{
		int parsePos = startPos+i;
		[branch]
		if (parsePos < inputBufferSize)
		{
			uint3 input = inLData[parsePos];
			[branch]
			switch(input.x)
			{
				case 91: // [
					{
						if (depthPos == 0)
						{
							uint p = depthCounters[depthPointer + gOffset] + dSums[groupPointer*depthTotal];
							//if (dtrue)
								brackets[p] = set7Int(parsePos, 0, depthCounters[depthPointer + gOffset + GROUPDIMX] + dSums[1+(groupPointer*depthTotal)], 0, groupPointer, vers, depthPos );
							//else
							//	brackets[p] = set7Int(parsePos, 0, 0, 0, 0, vers, depthPos );
						}
						else
						{
							uint p = depthCounters[depthPointer+(depthPos*GROUPDIMX) + gOffset] + dSums[depthPos + (groupPointer*depthTotal)] + depthSizes.Load((depthPos-1)*12);
							if (depthPos == depthTotal-1)
								brackets[p] = set7Int(parsePos, depthCounters[depthPointer+((depthPos-1)*GROUPDIMX)+gOffset] + dSums[(depthPos-1) + (groupPointer*depthTotal)], 0, 0, groupPointer/*depthSizes.Load((depthPos-1)*12)*/, vers, depthPos );
							else
								brackets[p] = set7Int(parsePos, depthCounters[depthPointer+((depthPos-1)*GROUPDIMX)+gOffset] + dSums[(depthPos-1) + (groupPointer*depthTotal)], depthCounters[depthPointer+((depthPos+1)*GROUPDIMX)+gOffset] + dSums[(depthPos+1) + (groupPointer*depthTotal)], 0, groupPointer, vers, depthPos );
						}
						depthCounters[depthPointer+(depthPos*GROUPDIMX) + gOffset]++;
						depthPos++;
					}
					break;
				case 93: // ]
					{
						depthPos--;
						if (depthPos == 0 )
						{
							uint p = depthCounters[depthPointer + gOffset] + dSums[groupPointer*depthTotal];
							//if (dtrue)
								brackets[p] = set7Int(parsePos, 0, depthCounters[depthPointer + gOffset + GROUPDIMX] + dSums[1+(groupPointer*depthTotal)], 0, groupPointer, vers, depthPos );
							//else
							//	brackets[p] = set7Int(parsePos, 0, 0, 0, 0, 0, depthPos);
						}
						else
						{
							uint p = depthCounters[depthPointer+(depthPos*GROUPDIMX) + gOffset] + dSums[depthPos + (groupPointer*depthTotal)] + depthSizes.Load((depthPos-1)*12);
							if (depthPos == depthTotal-1)
								brackets[p] = set7Int(parsePos, depthCounters[depthPointer+((depthPos-1)*GROUPDIMX)+gOffset] + dSums[(depthPos-1) + (groupPointer*depthTotal)], 0, 0, groupPointer /*depthSizes.Load((depthPos-1)*12)*/, vers, depthPos );
							else
								brackets[p] = set7Int(parsePos, depthCounters[depthPointer+((depthPos-1)*GROUPDIMX)+gOffset] + dSums[(depthPos-1) + (groupPointer*depthTotal)], depthCounters[depthPointer+((depthPos+1)*GROUPDIMX)+gOffset] + dSums[(depthPos+1) + (groupPointer*depthTotal)], 0, groupPointer, vers, depthPos );
						}
						//brackets[p].z = parsePos;
						//brackets[p].q = vers;
						depthCounters[depthPointer+(depthPos*GROUPDIMX) + gOffset]++;
					}
					break;
				default:
					//*
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

	if (parseStartA < inputBufferSize)
	{
		int4 counts = countVals[ai];
		depthBracketCount(GTid.x,Gid.x,parseStartA,counts);
	}

	if (parseStartB < inputBufferSize)
	{
		int4 counts = countVals[bi];
		depthBracketCount(GTid.x+THREADS,Gid.x,parseStartB,counts);
	}
}