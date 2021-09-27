//--------------------------------------------------------------------------------------
// File: ParserCompute5.1.hlsl
//
// L-System parser
//--------------------------------------------------------------------------------------

#define GROUPDIMX 256

cbuffer buffSize
{
	uint inputBufferSize;
	uint depthTotal;
	uint numBr;
	uint currDepth;
}

RWStructuredBuffer<uint>	dSums			: register(u3);
RWByteAddressBuffer			depthSizes		: register(u5);

[numthreads(GROUPDIMX, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint size;
	uint blank;
	dSums.GetDimensions(size,blank);
	size = size/depthTotal;

	uint depthBucketCounter = 0;
	if (GTid.x < depthTotal)
	{
		uint counter = 0;
		[loop]
		for (int a = 0; a < size; a++) // build sum in place up the tree 
		{
			uint temp = dSums[GTid.x+(a*depthTotal)];
			dSums[GTid.x+(a*depthTotal)] = counter;
			counter += temp;
		}
		uint3 test = uint3(counter,0,0);
		depthSizes.Store3(GTid.x*12, test);
	}

	GroupMemoryBarrierWithGroupSync();

	if(GTid.x == 0)
	{
		uint counter = 0;
		[loop]
		for (int i = 0; i < depthTotal; i++) // build sum in place up the tree 
		{
			uint temp = depthSizes.Load(i*12);
			counter += temp;
			uint3 test = uint3(counter,0,0);
			depthSizes.Store3(i*12, test);
		}
	}

	/*
	uint groupOffset = Gid.x*GROUPDIMX;
	uint ai = GTid.x;//+groupOffset;
	uint bi = ai + THREADS;
	uint bankOffsetA = CONFLICT_FREE_OFFSET(GTid.x); 
	uint bankOffsetB = CONFLICT_FREE_OFFSET(GTid.x + THREADS);
	uint localA = GTid.x + bankOffsetA;
	uint localB = (GTid.x + THREADS) + bankOffsetB;

	uint size;
	uint blank;
	countVals.GetDimensions(size,blank);
	size = (size/GROUPDIMX)+1;

	uint depthBucketCounter = 0;
	[loop]
	for(uint i=0; i<depthTotal;i++)
	{
		if (ai < size)
		{
			prefixSum[localA] = dSums[i+(ai*depthTotal)];
		}
		else
			prefixSum[localA] = 0;

		if (bi < size)
		{
			prefixSum[localB] = dSums[i+(bi*depthTotal)];
		}
		else
			prefixSum[localB] = 0;

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
			uint end = prefixSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];
			depthBucketCounter += end;
			uint3 test = uint3(depthBucketCounter,0,0);
			depthSizes.Store3(i*12, test);
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

		if (ai < size)
		{
			 dSums[i+(ai*depthTotal)] = prefixSum[localA];
		}

		if (bi < size)
		{
			dSums[i+(bi*depthTotal)] = prefixSum[localB];
		}
	}
	//*/
}