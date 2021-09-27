//--------------------------------------------------------------------------------------
// File: LSys.hlsl
//
// L-System parser. Outputs result
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
#define LOG_NUM_BANKS 4 
 
/*#ifdef ZERO_BANK_CONFLICTS 
#define CONFLICT_FREE_OFFSET(n) \ 
 ((n) >> NUM_BANKS + (n) >> (2 * LOG_NUM_BANKS)) 
#else */
#define CONFLICT_FREE_OFFSET(n) ((n) >> LOG_NUM_BANKS) 
//#endif

struct sixUINT
{
	uint a;
	uint b;
	uint c;
	uint d;
	uint e;
	uint f;
};

cbuffer buffSize{
	uint ruleBufferSize;
	uint tableBufferSize;
	uint inputBufferSize;
	float timeIncre;
}

StructuredBuffer<uint>  ruleData    : register(t0);
StructuredBuffer<uint>  ruleOffSets : register(t1);

//LData x-action y-paramPtr z-paramNum
RWStructuredBuffer<uint3> inLData		 : register(u0);
RWStructuredBuffer<uint3> outputLData    : register(u1);
RWStructuredBuffer<float> paramData      : register(u2);
RWStructuredBuffer<float> paramDataRW    : register(u3);
RWStructuredBuffer<uint2> sums           : register(u4);
RWStructuredBuffer<uint2>  dataSize      : register(u5);
RWStructuredBuffer<sixUINT> rulePointers : register(u6);
RWStructuredBuffer<uint2> rewriteOutput  : register(u7);

groupshared uint2 rewriteMax[THREADS];

float sum(float in1, float in2, uint sumType)
{
	float output;
	//[branch]
	[flatten]
	switch(sumType)
	{
		case 42:
		{
			output = in1*in2;
		}
		break;
		case 43:
		{
			output = in1+in2;
		}
		break;
		case 45:
		{
			output = in1-in2;
		}
		break;
		case 47:
		{
			output = in1/in2;
		}
		break;
		default:
		{
			output = 0;
		}
		break;
	}
	return output;
}

uint numberLoop(inout uint iterPoint, uint size)
{
	uint integerPart = 0;
	uint r = 0;//ruleData[iterPoint];
	[loop]
	for (iterPoint; iterPoint < size; iterPoint++)
	{
		r = ruleData[iterPoint];
		if (r >= 48 && r <= 57)
			integerPart = integerPart*10 + (r - 48);
		else
			return integerPart;
	}
	return integerPart;
}

float findValue(inout uint iterPoint, uint size, uint paramPtr)
{
	uint sumType = 0;
	float value1 = -1.f;
	float value2 = -1.f;
	bool firstValSearch = true;
	uint r;
	///*
	[loop]
	for (; iterPoint<size; iterPoint++)
	{
		r = ruleData[iterPoint];
		if (firstValSearch)
		{
			firstValSearch = false;
			if (r >= 48 && r <= 57/* |  (r == 45)*/)
			{
				uint ps = numberLoop(iterPoint, size);
				if (ps < tableBufferSize)
					value1 = paramData[ps];
				else
					value1 = paramData[paramPtr+(ps-tableBufferSize)];
				iterPoint--;
			}
			else{
				//If reached - Bad things have happened
				return value1;
			}
		}
		// * + - /
		else if (r == 42 | r == 43 | r == 45 | r == 47)
		{
			sumType = r;
		}
		else if (r >= 48 && r <= 57/* | (r == 45)*/)
		{
			uint ps = numberLoop(iterPoint, size);
			if (ps < tableBufferSize)
				value2 = paramData[ps];
			else
				value2 = paramData[paramPtr+(ps-tableBufferSize)];
			if (sumType != 0)
				value1 = sum(value1,value2,sumType);
			iterPoint--;
		}
		else if (r == 44)
		{ 
			return value1; 
		}
		else{
			return value1;
		}
	}
	return value1;
}

uint3 setLData(inout uint iterPoint, uint size, uint paramPtr, inout uint paramRWPtr)
{
	uint3 output;
	uint r = ruleData[iterPoint];
	output.x = r;
	output.y = paramRWPtr;
	output.z = 0;

	//26.03.15 added.
	if (r == 91 || r == 93)
	{
		paramDataRW[paramRWPtr] = 0;
		output.y = paramRWPtr;
		output.z = 1;
		iterPoint++;
		paramRWPtr++;
		return output;
	}
	
	if (iterPoint+1 < size)
	{
		iterPoint++;
		r = ruleData[iterPoint];
		if (r == 40)
		{
			uint passOnSize = size;
			//*
			[loop]
			for(uint i = iterPoint+1; i<size; i++)
			{
				r = ruleData[i];
				if (r == 41){
					passOnSize = i;
					break;
				}
				if (r == 40)
					break;

			}
			iterPoint++;
			uint paramCount = 0;

			[loop]
			while (iterPoint < passOnSize)
			{
				float temp;
				paramDataRW[paramRWPtr+paramCount] = findValue(iterPoint, passOnSize, paramPtr);
				paramCount++;
				iterPoint++;
			}
			//*/
			
			output.y = paramRWPtr;
			output.z = paramCount;
			paramRWPtr = (paramRWPtr+paramCount);
			return output;
		}else{
			return output;
		}
	}
	iterPoint++;
	return output;
}

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint pos = Gid.x;
	uint groupOffset = Gid.x*THREADS;
	uint thread = DTid.x+groupOffset;/*GTid.x+groupOffset;*/
	uint ai = thread;
	uint bi = thread + THREADS;

	uint aiSize = dataSize[ai].x;
	uint biSize = dataSize[bi].x;
	uint apPos = dataSize[ai].y;
	uint bpPos = dataSize[bi].y;

	[loop]
	for (int i=0; i<pos;i++)
	{
		uint2 iter = sums[i];
		aiSize += iter.x;
		biSize += iter.x;
		apPos += iter.y;
		bpPos += iter.y;
	}
	dataSize[ai].x = aiSize;
	dataSize[bi].x = biSize;
	dataSize[ai].y = apPos;
	dataSize[bi].y = bpPos;

	//uint bankOffsetA = CONFLICT_FREE_OFFSET(GTid.x); 
	//uint bankOffsetB = CONFLICT_FREE_OFFSET(GTid.x + THREADS);
	//uint localA = GTid.x + bankOffsetA;
	//uint localB = (GTid.x + THREADS)+ bankOffsetB;
	uint3 aLData = (0,0,0);
	uint3 bLData = (0,0,0);
	aLData = inLData[ai];
	bLData = inLData[bi];
	rewriteMax[GTid.x].x = 0;
	rewriteMax[GTid.x].y = 0;
	uint rwt = 0;
	uint rwp = 0;
	AllMemoryBarrierWithGroupSync();
	//*
	if (ai < inputBufferSize)
	{
		uint threadRewritePos  = rulePointers[ai].a;
		uint threadRewriteSize = rulePointers[ai].b;
		if (threadRewritePos > 0)
		{
			[allow_uav_condition]
			for (threadRewritePos; threadRewritePos<threadRewriteSize; threadRewritePos)
			{
				uint3 test = setLData(threadRewritePos, threadRewriteSize, aLData.y, apPos );
				outputLData[aiSize] = test;
				aiSize++;
			}
			rwt += rulePointers[ai].e;
			rwp += rulePointers[ai].f;
		}
		else
		{
			rwt++;
			rwp += aLData.z;
			uint pSize = aLData.y+aLData.z;
			uint apBef = apPos;
			[allow_uav_condition]
			for(uint i=aLData.y;i<pSize;i++)
			{
				paramDataRW[apPos] = paramData[i];
				apPos++;
			}
			aLData.y = apBef;
			outputLData[aiSize] = aLData;
		}
	}
	//*/
	//*
	if (bi < inputBufferSize)
	{
		uint thread2RewritePos  = rulePointers[bi].a;
		uint thread2RewriteSize = rulePointers[bi].b;
		if (thread2RewritePos > 0)
		{
			[allow_uav_condition]
			for (thread2RewritePos; thread2RewritePos<thread2RewriteSize; thread2RewritePos)
			{
				uint3 test = setLData(thread2RewritePos, thread2RewriteSize, bLData.y, bpPos );
				outputLData[biSize] = test;
				biSize++;
			}
			rwt += rulePointers[bi].e;
			rwp += rulePointers[bi].f;
		}
		else
		{
			rwt++;
			rwp += bLData.z;
			uint pSize = bLData.y+bLData.z;
			uint bpBef = bpPos;
			[allow_uav_condition]
			for(uint i=bLData.y;i<pSize;i++)
			{
				paramDataRW[bpPos] = paramData[i];
				bpPos++;
			}
			bLData.y = bpBef;
			outputLData[biSize] = bLData;//uint3(bLData.x,bLData.y,bLData.z);//uint3(0,0,0);//bLData;
		}
	}
	//*/
	//*
	rewriteMax[GTid.x].x = rwt;
	rewriteMax[GTid.x].y = rwp;
	AllMemoryBarrierWithGroupSync();
	if (GTid.x == 0)
	{
		//A horible thing
		unsigned int r = 0;
		unsigned int p = tableBufferSize;
		[unroll(THREADS)]
		for(int i = 0; i<THREADS; i++)
		{
			r += rewriteMax[i].x; 
			p += rewriteMax[i].y; 
		}
		rewriteOutput[Gid.x].x = r;
		rewriteOutput[Gid.x].y = p;
	}
	//*/
}