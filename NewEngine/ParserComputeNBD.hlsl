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
	uint dSize;
	uint numBr;
	uint currDepth;
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

StructuredBuffer<uint3>			inLData			: register(t1);

//StructuredBuffer<uint3>		inLData			: register(t1);
RWStructuredBuffer<float>	paramData	    : register(u0);
RWStructuredBuffer<uint7>	sums			: register(u1);
//RWStructuredBuffer<int4>	countVals		: register(u2);
RWByteAddressBuffer			depthSizes		: register(u5);
RWStructuredBuffer<uint7>	brackets		: register(u6);

uint7 set7Int(uint a, uint b, uint c, uint d, uint e, uint f, uint g)
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

groupshared uint preSum[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint posA = GTid.x;
	uint posB = (GTid.x+THREADS);
	uint localA = (posA) + CONFLICT_FREE_OFFSET(posA);
	uint localB = (posB) + CONFLICT_FREE_OFFSET(posB);

	uint7 fBr;
	uint7 lBr;
	
	uint max = GROUPDIMX*2;
	uint aT = (posA+1)*2;//((posA+1)*2)+1;
	uint bT = (posB+1)*2;//((posB+1)*2)+1;
	uint mOffset = 0;

	uint dS = 0;
	if (Gid.x == 0)
	{
		max = depthSizes.Load(/*12*/0);
		fBr = set7Int(0, -1, 0, 0, 0, 0, 0);
		mOffset = max;//depthSizes.Load(/*12*/0);
		lBr = set7Int(inputBufferSize, 0, mOffset, 0, 0, 0, 0);
		sums[Gid.x] = set7Int(0,0,0,0,0,0,0);
	}
	else if ((Gid.x-1)*2 < numBr)
	{
		sums[Gid.x] = set7Int(0,0,0,0,0,0,0);
		uint brP = (Gid.x-1)*2;
		fBr = brackets[brP];
		lBr = brackets[brP+1];

		//Param set so that the parser can skip brackets
		paramData[inLData[fBr.x].y]=brP+1;

		fBr.x++;
		sums[Gid.x].w = (brP/2);

		max = (lBr.z - fBr.z);
		mOffset = 0;

		dS = fBr.q+1;

		// the + fbr.z should maybe be removed and the brackets on creation should contain the correct offsets
		if( fBr.q == dSize-1 )
			mOffset = numBr + fBr.z;
		else
			mOffset = depthSizes.Load(/*(*/fBr.q/*+1)*/*12) + fBr.z;

		//11/03/2015 part of the fix attempt
		if (max == 0)
			max += 2;

		aT += mOffset;
		bT += mOffset;
		max += mOffset;
	}

	if (GTid.x == 0)
	{
		sums[Gid.x].x = dS;//fBr.y+1;
		sums[Gid.x].y = lBr.z - fBr.z;
		sums[Gid.x].z = mOffset;
	}

	uint groupTotal = -1;
	//uint7 s = set7Int(0, 0,0, 0, 0, 0,0);

	//s.w = lBr.x
	//s.z = fBr.x
	if (aT <= max)
	{
		//We need to rethink this entire algorithm this is so fucking bad
		uint aRead = aT-2;

		uint spa = brackets[aRead].x;
		if (aT == max)
		{
			if ((lBr.z - fBr.z) == 0)
				groupTotal = 0;//lBr.x - (fBr.x +1);
			else
			//+1 to remove positional. ie, nothing between 4 and 5.
				groupTotal = lBr.x - (brackets[aRead+1].x+1);
			//groupTotal = lBr.x - (brackets[(s.x+aT)+1].x+1);
			//groupTotal = (s.w - brackets[(s.x+aT)+1].x)+1;
			//sums[Gid.x].w = groupTotal;
		}

		if (posA == 0)
		{
			if ((lBr.z - fBr.z) == 0)
				spa = lBr.x-1;
			preSum[0] = (spa - fBr.x)+1;
		}
		else
			preSum[localA] = spa - brackets[aRead-1].x/*)+1*/;
	}
	else
		preSum[localA] = 0;

	if (bT <= max)
	{
		preSum[localB] = 0;// brackets[bT].x - brackets[bT-1].x;
		//if (bT == max)
		//	groupTotal = lBr.x - (brackets[bT+1].x+1);
	}
	else
		preSum[localB] = 0;

	GroupMemoryBarrierWithGroupSync();
	//if (GTid.x == 0)
	//	sums[1] = set7Int(preSum[0],preSum[1],preSum[2],preSum[3],preSum[4],preSum[32],99);
	///*
	//PREFIX SUM
	uint offset = 1;
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
			preSum[bi2] += preSum[ai2];
		}
		offset *= 2;
	}
	GroupMemoryBarrierWithGroupSync();

	if (groupTotal != -1)
	{
		/*
		if (Gid.x == 2 && dSize == 3)
		{
			sums[Gid.x].x = fBr.x;
			sums[Gid.x].y = lBr.x;
			sums[Gid.x].z = aT;
			sums[Gid.x].w = max;
			sums[Gid.x].v = groupTotal;
		}
		//*/

		groupTotal += preSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];
		InterlockedAdd( sums[Gid.x].u, groupTotal );
		InterlockedAdd( sums[Gid.x].q, groupTotal/PARSECHUNK/**/ );
		////sums[2] = set7Int(preSum[0],preSum[1],preSum[2],preSum[3],preSum[4],preSum[32],99);
		////If the parse blocks use up all the data perfectly, then we don't need an extra parse.
		if (groupTotal%PARSECHUNK != 0)
			InterlockedAdd( sums[Gid.x].q, 1);
	}
}