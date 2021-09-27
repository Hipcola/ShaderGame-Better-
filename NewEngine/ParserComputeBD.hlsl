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
	uint blank;
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

//StructuredBuffer<uint3>		inLData			: register(t1);
//RWStructuredBuffer<float>	paramData	    : register(u0);
RWStructuredBuffer<uint7>	sums			: register(u1);
RWStructuredBuffer<int4>	countVals		: register(u2);
RWByteAddressBuffer			depthSizes		: register(u5);
RWStructuredBuffer<uint7>	brackets		: register(u6);
RWStructuredBuffer<uint7>	nSums			: register(u7);

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
	uint7 s = sums[Gid.x];
	//s.x = s.x*32;
	//uint max	 = (s.y-s.x)/2;
	uint max = GROUPDIMX*2;
	uint soffset = s.v;
	//uint goffset  = s.v * 32;

	uint posA = GTid.x/* + goffset*/;
	uint posB = (GTid.x+THREADS) /*+ goffset*/;
	uint localA = (posA) + CONFLICT_FREE_OFFSET(posA);
	uint localB = (posB) + CONFLICT_FREE_OFFSET(posB);

	uint7 fBr;
	uint7 lBr;

	uint3 endVal = uint3(-1,-1,-1);
	uint aT = (posA+1)*2;
	uint bT = (posB+1)*2;

	if (Gid.x == 0)
	{
		fBr = set7Int(0, 0, 0, 0, 0, 0, 0);
		lBr = set7Int(inputBufferSize, 0, 0, 0, 0, 0, 0);
		max = s.z/*/2*/;
	}
	else if ((Gid.x-1)*2 < numBr)
	{
		uint brP = (Gid.x-1)*2;
		fBr = brackets[brP];
		lBr = brackets[brP+1];
		fBr.x++;
		max = lBr.z - fBr.z;
		//lBr.x++;
		if (max == 0)
			max += 2;

		aT += s.z;
		bT += s.z;
		max += s.z;
	}

	uint fpa=0;
	uint fpb=0;
	uint spa=0;
	uint spb=0;

	/*
	if( Gid.x == 1 && GTid.x == 0)
	{
		countVals[soffset].x = soffset;
		countVals[soffset].y = max;
		countVals[soffset].z = posA;
		countVals[soffset].y = aT;
	}*/

	if (aT <= max)
	{
		//We need to rethink this entire algorithm this is so fucking bad
		uint aRead = aT-2;

		spa = brackets[aRead].x;
		if (aT == max)
		{
			//23.03.15
			// Ok, so before this change, endval.z, was set to -1, because we didn't have any 'end' data to read.
			// But end 'now' is also the only point where the actual total data to be read is set.
			// changes to the counting means the total in 'posA == 0' or whatever is one less than the total, due to the starting position.
			// Now we set to make .x = 0, allowing end data to be entered, but it should never write any positions, it's just so the sums can be passed over.
			// This is bad
			// Really we need to look at the prefix sum and why we don't do that in the NBD pass
			int forwardCheck = lBr.x-1;
			if (s.y != 0)
				//+1 to remove positional. ie, nothing between 4 and 5.
				forwardCheck = brackets[aRead+1].x;

			endVal.x = lBr.x - (forwardCheck+1);
			endVal.z = forwardCheck;
			//preSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)] = s.w - spa.x;
		}

		if (posA == 0)
		{
			//*
			//if(Gid.x == 1){
			countVals[soffset].x = fBr.x;
			countVals[soffset].y = Gid.x;
			countVals[soffset].z = -1;
			countVals[soffset].w = aT;
			//}
			//*/
			fpa = fBr.x;
			//If these brackets contain no other brackets then the entire section is processed here in this thread.
			if (s.y == 0)
				spa = lBr.x-1;
			preSum[0] = (spa - fBr.x)+1;
		}
		else
		{
			//brackets[start+aT].v = 99;//(start+aT)-1;
			fpa = brackets[aRead-1].x;
			preSum[localA] = spa - fpa/*)+1*/;
			//fpa++;
			//spa.w = v;
		}
	}
	else
		preSum[localA] = 0;

	//preSum[localB] = 0;
	if (bT <= max)
	{
		//spb = brackets[bT].x;
		//fpb = brackets[bT-1].x;
		preSum[localB] = 0;//spb - fpb;
		/*if (bT == max)
		{
			int forwardCheck = brackets[aT+1].x + 1;
			endVal.x = lBr.x - forwardCheck;
			endVal.z = forwardCheck;
			//preSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)] = s.w - spb.x;
		}*/
	}
	else
		preSum[localB] = 0;

	GroupMemoryBarrierWithGroupSync();

	/*
	countVals[GTid.x].x = preSum[localA];
	countVals[GTid.x].y = bankOffsetA;
	countVals[GTid.x].z = localA;
	countVals[t2].x = preSum[localB];
	countVals[t2].y = GROUPDIMX+(GROUPDIMX/NUM_BANKS);
	countVals[t2].z = localB;
	//*/

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
	// clear the last element 
	if (endVal.x != -1)
		endVal.y = preSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];
	GroupMemoryBarrierWithGroupSync();
	///*
	if (GTid.x == 0)
	{
		//InterlockedAdd( sums[Gid.x].q, (endVal.y/PARSECHUNK)+1);
		preSum[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)] = 0;
	}

	[loop]
	for (int b = 1; b < GROUPDIMX; b *= 2) // traverse down tree & build scan 
	{ 
		offset >>= 1; 
		GroupMemoryBarrierWithGroupSync();
		if (GTid.x < b)
		{ 
			int aii = offset*(2*GTid.x+1)-1;
			int bii = offset*(2*GTid.x+2)-1;
			aii += CONFLICT_FREE_OFFSET(aii);
			bii += CONFLICT_FREE_OFFSET(bii);
			uint l = preSum[aii];
			preSum[aii] = preSum[bii];
			preSum[bii] += l;
		}
	}
	//*/
	//GroupMemoryBarrierWithGroupSync();

	///*
	if (aT <= max /*&& Gid.x == 1*/)
	{
		uint val = preSum[localA];
		uint start = val/PARSECHUNK;
		uint total = val + (spa - fpa);
		uint Nstart = total/PARSECHUNK;
		//if (Gid.x == 0 && GTid.x == 1)
		//	countVals[1] = uint4(val,endVal.x,endVal.z,endVal.y);
		
		//23.03.2015 Moved here from end.
		// When the parse is = to PARSECHUNK, we found it wrote the last positon, being a bracket.
		// so say -> 7 [ ] 9 END. We'd write the last parse positon at [. When it should start at ], meaning END will write it.
		// TESTED. Doesn't work anyway. As end reads 8 to be the last write, it considers a parse position to be set.
		/*if (start == (Nstart-1))
		{
			uint modCheck = total%PARSECHUNK;
			if (modCheck == 0)
				Nstart = 0;
		}*/

		//if(total == s.u)
		//	nSums[Gid.x] = s;

		/*
		if(Gid.x == 3 && dSize == 3)
		{
			//s.x = aT;
			//s.y = max;
			//s.w = spa;
			//s.u = endVal.x;
			s.v = total;//endVal.y;
			//s.q = endVal.z;
			nSums[Gid.x] = s;
		}*/

		uint modCheck = total%PARSECHUNK;
		if (modCheck == 0)
			Nstart--;

		uint modulus = val%PARSECHUNK;
		if (modulus == 0 && val > 0)
		{				
			fpa++;
			countVals[start+soffset].x = fpa;
			countVals[start+soffset].y = Gid.x;
			countVals[start+soffset].z = 1;
			countVals[start+soffset].w = 99;
		}else
		{
			//start++;
			modulus = PARSECHUNK - modulus;
			fpa = (fpa + modulus)+1;
			/*
			countVals[start+soffset].x = fpa;
			countVals[start+soffset].y = Gid.x;
			countVals[start+soffset].z = val;
			countVals[start+soffset].w = 99;
			*/
		}

		uint i = 0;
		//*
		[loop]
		for(;start<Nstart;)
		{
			start++;
			countVals[start+soffset].x = fpa + (i*PARSECHUNK);
			countVals[start+soffset].y = Gid.x;
			countVals[start+soffset].z = 2;
			countVals[start+soffset].w = total;
			i++;
		}
	}
	//*/

	/*
	if (bT <= max)
	{
		uint val = preSum[localB];
		uint start = val/PARSECHUNK;
		uint total = val + (spa - fpb);
		uint Nstart = total/PARSECHUNK;
		if (Nstart > start)
		{
			uint modulus = val%PARSECHUNK;
			if (modulus != 0)
			{
				start++;
				modulus = PARSECHUNK - modulus;
			}
			fpb.x = (fpb.x + modulus)+1;
			countVals[start+soffset].x = fpb;
			countVals[start+soffset].y = -1;
			countVals[start+soffset].z = 2;
			uint i = 1;
			[loop]
			for(;start<Nstart;)
			{
				start++;
				countVals[start+soffset].x = fpb + (i*PARSECHUNK);
				countVals[start+soffset].y = -1;
				countVals[start+soffset].z = 1;
				i++;
			}
		}
	}
	//*/
	///*
	if (endVal.z != -1/* && Gid.x == 1*/)
	{
		//*
		uint start  = endVal.y/PARSECHUNK;
		uint total  = endVal.y + endVal.x;
		uint Nstart = total/PARSECHUNK;

		//15.03.2015 Fix for writing when total is equal to parsechunk ie, 8 items
		/*
		if (start == (Nstart-1))
		{
			uint modCheck = total%PARSECHUNK;
			if (modCheck == 0)
				Nstart = 0;
		}
		//*/

		//Needs to be updated
		if(total == s.u)
			nSums[Gid.x] = s;

		uint modCheck = total%PARSECHUNK;
		if (modCheck == 0)
			Nstart--;

		uint modulus = endVal.y%PARSECHUNK;
		if (modulus == 0 && endVal.y > 0)
		{				
			endVal.z++;
			countVals[start+soffset].x = endVal.z;
			countVals[start+soffset].y = Gid.x;
			countVals[start+soffset].z = 3;
			countVals[start+soffset].w = 99;
		}else
		{
			//start++;
			modulus = PARSECHUNK - modulus;
			endVal.z = (endVal.z  + modulus)+1;
			/*
			countVals[start+soffset].x = fpa;
			countVals[start+soffset].y = Gid.x;
			countVals[start+soffset].z = val;
			countVals[start+soffset].w = 99;
			*/
		}

		uint i = 0;
		//*
		[loop]
		for(;start<Nstart;)
		{
			start++;
			countVals[start+soffset].x = endVal.z + (i*PARSECHUNK);
			countVals[start+soffset].y = Gid.x;
			countVals[start+soffset].z = 4;
			countVals[start+soffset].w = total;
			i++;
		}
	}
	//*/
}