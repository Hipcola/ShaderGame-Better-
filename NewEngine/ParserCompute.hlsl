//--------------------------------------------------------------------------------------
// File: ParserCompute.hlsl
//
// The First stage of the parse system simply counts the vertices that will need to be stored and
// the 'bracket values', where a push '[' is +1 and a pop ']' is -1.
//--------------------------------------------------------------------------------------

//ASCII Stuff
//( - 40
//) - 41
//, - 44
//0 - 48
//9 - 57
//A - 65
//Z - 90
//a - 97
//z - 122

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
	uint buff1;
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

StructuredBuffer<uint>  drawChars		 : register(t0);
StructuredBuffer<uint3>  inLData		 : register(t1);

//RWStructuredBuffer<float>	paramData	 : register(u0);
RWStructuredBuffer<uint7>		sums	 : register(u1);
RWStructuredBuffer<int4>	countVals	 : register(u2);
/*
StructuredBuffer<uint>  drawChars		 : register(t0);
StructuredBuffer<uint3>  inLData		 : register(t1);
RWStructuredBuffer<float4>	vertices		: register(u0);
RWStructuredBuffer<uint>	indices			: register(u1);
RWStructuredBuffer<int4>	sums			: register(u2);
RWStructuredBuffer<int4>	countVals		: register(u3);
RWStructuredBuffer<uint>	depthCounters	: register(u4);
RWStructuredBuffer<uint2>	brackets		: register(u5);
RWStructuredBuffer<uint3>	depthSizes		: register(u6);
RWStructuredBuffer<uint3>	spacings		: register(u7); //x-Pos,y-Dom,z-Low
*/

groupshared int totalVers[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared int bracketVal[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared int bracketCount[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared int bracketMax[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];

int6 set6Int(int a, int b, int c, int d, int e, int f)
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

void countPreBuild(uint startPos, uint localMem)
{
	uint dcSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);
	//uint startParse = startPos;//*PARSECHUNK;
	int vers = 0;
	int bv = 0;
	int bc = 0;
	int bm = 0;
	[loop]
	for (uint i = 0; i<PARSECHUNK; i++)
	{
		//bv++;
		int parsePos = startPos+i;
		[branch]
		if (parsePos < inputBufferSize)
		{
			uint3 input = inLData[parsePos];
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
					ptr *= 2;
				else if (input.x < lookup && (ptr/2) >= 0)
					ptr >>=1;
			}

			[branch]
			if (drawCheck)
			{
				vers++;
			}
			else
			{
				[branch]
				switch(input.x)
				{
					case 91: // [
						bv++;
						bc++;
						if (bv > bm)
							bm = bv;
						break;
					case 93: // ]
						bv--;
						bc++;
						break;
					default:
						break;
						;
				}
			}
		}else{
			i = PARSECHUNK;
		}
	}
	totalVers[localMem]		= vers;
	bracketVal[localMem]	= bv;
	bracketCount[localMem]	= bc;
	bracketMax[localMem]	= bm;
	//orientations[localMem] = localOrien;
	//posLocal[localMem] = pos;
}

//GROUPDIMX
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
	uint end = GROUPDIMX-1+CONFLICT_FREE_OFFSET(GROUPDIMX-1);
	int aBracketCount = 0;
	int bBracketCount = 0;

	totalVers[localA] = 0;
	totalVers[localB] = 0;
	bracketVal[localA] = 0;
	bracketVal[localB] = 0;
	bracketCount[localA] = 0;
	bracketCount[localB] = 0;
	bracketMax[localA] = 0;
	bracketMax[localB] = 0;

	if(parseStartA<inputBufferSize)
	{
		//int2 aRes = countPreBuild(parseStartA,localA);
		countPreBuild(parseStartA,localA);//aRes.x;
	}

	if(parseStartB<inputBufferSize)
	{
		//int2 bRes = countPreBuild(parseStartB,localB);
		countPreBuild(parseStartB,localB);//bRes.x;
	}

	///*
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

			totalVers[bi2]  += totalVers[ai2];
			bracketVal[bi2] += bracketVal[ai2];
			bracketCount[bi2] += bracketCount[ai2];
			/*if (bracketMax[ai2] > bracketMax[bi2])
				bracketMax[bi2] = bracketMax[ai2];
			else
				bracketMax[ai2] = bracketMax[bi2];*/
			//orientations[bi2] = mul(orientations[ai2],orientations[bi2]);
			//posLocal[bi2].xyz += posLocal[ai2].xyz;
		}
		offset *= 2;
	}
	
	GroupMemoryBarrierWithGroupSync();
	// clear the last element 
	if (GTid.x == 0)
	{
		int tv = totalVers[end];
		int bc = bracketCount[end];
		int bv = bracketVal[end];
		int bm = bracketMax[end];
		totalVers[end]		= 0;
		bracketVal[end]		= 0;
		bracketCount[end]	= 1;//1;
		sums[Gid.x] = set7Int(tv,/*bm*/0,bc,bv,0,0,0); 

		//unsigned int c = 0;
		//sums[Gid.x+(c/4)] = int4(bracketMax[c],bracketMax[c+1],bracketMax[c+2],bracketMax[c+3]); 
		//c = c+4;
		//sums[Gid.x+(c/4)] = int4(bracketMax[c],bracketMax[c+1],bracketMax[c+2],bracketMax[c+3]); 
		//c = c+4;
		//sums[Gid.x+(c/4)] = int4(bracketMax[c],bracketMax[c+1],bracketMax[c+2],bracketMax[c+3]); 
		//c = c+4;
		//sums[Gid.x+(c/4)] = int4(bracketMax[c],bracketMax[c+1],bracketMax[c+2],bracketMax[c+3]); 
		//c = c+4;
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

			int t = totalVers[aii];
			totalVers[aii] = totalVers[bii];
			totalVers[bii] += t;

			int b = bracketVal[aii];
			bracketVal[aii] = bracketVal[bii];
			bracketVal[bii] += b;

			int bc = bracketCount[aii];
			bracketCount[aii] = bracketCount[bii];
			bracketCount[bii] += bc;			
			//float3x3 or = orientations[aii];
			//orientations[aii] = orientations[bii];
			//orientations[bii] = mul(or,orientations[bii]);
			//float4 p = posLocal[aii];
			//posLocal[aii].xyz = posLocal[bii].xyz;
			//posLocal[bii].xyz += p.xyz;
			//if (p.w > 0)
			//	posLocal[bii].w = p.w;
		}
	}

	if( parseStartA < inputBufferSize )
		//countVals[ai] = set5Int(totalVers[localA],inputBufferSize,aBracketCount,aBracketVal,bracketVal[localA]);
		countVals[ai] = int4(totalVers[localA],inputBufferSize,bracketCount[localA],bracketVal[localA]);
	else
		countVals[ai] = int4(0,0,0,0);

	if( parseStartB < inputBufferSize )
		//countVals[bi] = set5Int(totalVers[localB],inputBufferSize,bBracketCount,bBracketVal,bracketVal[localB]/*,bracketCount[localB]*/);
		countVals[bi] = int4(totalVers[localB],inputBufferSize,bracketCount[localB],bracketVal[localB]);
	else
		countVals[bi] = int4(0,0,0,0);
	
	//To calculate the max depth we need to take the now summed val
	///*
	offset = 1;
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

			//Possible error causing
			//TODO
			int bma = bracketVal[ai2] + bracketMax[ai2];
			int bmb = bracketVal[bi2] + bracketMax[bi2];
			if (bma > bmb)
			{
				bracketMax[bi2] = bracketMax[ai2];
				bracketVal[bi2] = bracketVal[ai2];
			}
		}
		offset *= 2;
	}
	//*/
	GroupMemoryBarrierWithGroupSync();
	if (GTid.x == 0)
	{
		sums[Gid.x].y = bracketVal[end] + bracketMax[end];
		//sums[2] = set7Int(bracketVal[0],bracketMax[0],bracketVal[1],bracketMax[1],bracketVal[2],bracketMax[2],end); 
	}
}