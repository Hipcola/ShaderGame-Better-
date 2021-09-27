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

cbuffer buffSize
{
	uint ruleBufferSize;
	uint tableBufferSize;
	uint inputBufferSize;
	float timeIncre;
}

StructuredBuffer<uint>  ruleData    : register(t0);
StructuredBuffer<uint>  ruleOffSets : register(t1);

//LData x-action y-paramPtr z-paramNum
RWStructuredBuffer<uint3> inLData : register(u0);
RWStructuredBuffer<uint3> outputLData : register(u1);

//http://www.gamedev.net/topic/595036-rwstructuredbuffer-persistent-between-shaders/
RWStructuredBuffer<float> paramData   : register(u2);
RWStructuredBuffer<float> paramDataRW : register(u3);

RWStructuredBuffer<uint2> sums           : register(u4);
RWStructuredBuffer<uint2> dataSize       : register(u5);
RWStructuredBuffer<sixUINT> rulePointers : register(u6);
//RulePos
//RuleSize
//RewriteSize
//RewriteParamSize
//MaxRewriteSize
//MaxParamSize
RWStructuredBuffer<uint2> rewriteOutput : register(u7);

groupshared uint2 rewriteSize[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared uint2 rewriteMax[THREADS];
groupshared uint  rewriteFinalSize;

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

/*uint numberLoop2(inout uint iterPoint, uint size, inout bool numFound)
{
	uint integerPart = 0;
	uint r = 0;//ruleData[iterPoint];
	[loop]
	for (iterPoint; iterPoint < size; iterPoint++)
	{
		r = ruleData[iterPoint];
		if (r >= 48 && r <= 57)
		{
			integerPart = integerPart*10 + (r - 48);
		}
		else if(r == 40  && r == 44)
		{
			numFound = true;
			return integerPart;
		}
		else
		{
			numFound = false;
			return integerPart;
		}
	}
	return integerPart;
}*/

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
			//iterPoint++;
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
			//iterPoint++;
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
	output.z = 0/*paramRWPtr*/;

	//if a square bracket, bypass everything and auto set a blank param for later
	if (r == 91 || r == 93)
	{
		paramDataRW[paramRWPtr] = 0;
		output.y = paramRWPtr;
		output.z = 1;
		iterPoint++;
		paramRWPtr++;
		return output;
	}

	//Bracket check.
	//This is a messy fix for the 'size' issue.
	if (iterPoint+1 < size)
	{
		iterPoint++;
		r = ruleData[iterPoint];
		if (r == 40)
		{
			uint passOnSize = size;
			//*
			[loop]
			for (uint i = iterPoint+1; i<size; i++)
			{
				r = ruleData[i];
				if (r == 41){
					passOnSize = i;
					break;
				}
				if (r == 40)
					break;
					//If this is hit things are fucked
			}
			//*/
			iterPoint++;
			uint paramCount = 0;

			/*
			[loop]
			for (iterPoint; iterPoint<passOnSize; iterPoint++);
			{
				/*paramData[paramPtr+paramCount] = findValue(iterPoint, passOnSize, paramPtr);
				//output.y = iterPoint;
				paramCount++;
			}
			//*/

			//*
			[loop]
			while (iterPoint < passOnSize)
			{
				float temp;
				//temp = findValue(iterPoint, passOnSize, paramPtr);
				//paramData[paramPtr+paramCount] = findValue(iterPoint, passOnSize, paramPtr);
				/*paramDataRW[paramRWPtr+paramCount] = temp;*/
				paramDataRW[paramRWPtr+paramCount] = findValue(iterPoint, passOnSize, paramPtr);
				paramCount++;
				iterPoint++;
			}
			//*/
			
			output.y = paramRWPtr;
			output.z = paramCount;
			paramRWPtr = (paramRWPtr+paramCount);//+1;
			//iterPoint++;
			return output;
		}else{
			return output;
		}
	}
	iterPoint++;
	return output;
}

bool checkRule(uint iterPoint, uint paramPoint, uint size, float time)
{
	unsigned int localIter = iterPoint;
	localIter++;
	uint r = ruleData[localIter];
	//If there are parameters, we need to read the keys and check the time.
	if (r == 40)
	{
		localIter++;
		numberLoop(localIter,size);
		r = ruleData[localIter];
		if (r == 44)
		{
			localIter++;
			numberLoop(localIter,size);
			r = ruleData[localIter];
			if (r == 44)
			{
				localIter++;
				unsigned int tPtr = numberLoop(localIter,size);
				float rTime = paramData[tPtr];
				bool test1 = false;
				bool test2 = false;
				if (rTime <= time)
					test1 = true;
				if (tPtr >= tableBufferSize)
					test2 = true;
				bool test3 = test1 || test2;
				if (test3)
				//if (rTime <= time)
					return true;
				else
					return false;
			}
			else
				return true;
		}
		else
			return true;
	}
	else
		return true;
	return false;
}

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint groupOffset = Gid.x*THREADS;
	uint thread = /*GTid.x+groupOffset;*/DTid.x+groupOffset;
	uint ai = thread;
	uint bi = thread + THREADS;
	uint bankOffsetA = CONFLICT_FREE_OFFSET(GTid.x); 
	uint bankOffsetB = CONFLICT_FREE_OFFSET(GTid.x + THREADS);
	uint localA = GTid.x + bankOffsetA;
	uint localB = (GTid.x + THREADS) + bankOffsetB;
	//uint numRules = ruleBufferSize;

	rewriteSize[localA].x = 0;
	rewriteSize[localA].y = 0;
	rewriteSize[localB].x = 0;
	rewriteSize[localB].y = 0;

	uint threadRewritePos   = 0;
	uint threadRewriteSize  = 1;
	uint3 thisThreadInput   = (0,0,0);
	uint thread2RewritePos  = 0;
	uint thread2RewriteSize = 1;
	uint3 thisThread2Input  = (0,0,0);
	//float timeSig = 0.0f;
	//float timeSig2 = 0.0f;

	//Terrible.
	//Filling the starter data of the paramtable.
	if (Gid.x == 0 && GTid.x == 0)
	{
		[loop]
		for (int i = 0; i < tableBufferSize; i++)
		{
			paramDataRW[i] = paramData[i];
		}
	}

	if (ai < inputBufferSize)
	{
		//Preset the size to 1. If not rule matches the rewrite then we have to place the old uint2 back into the same position
		// which will still require 1 output buffer slot.
		rewriteSize[localA].x = 1;
		thisThreadInput = inLData[ai];
		rewriteSize[localA].y = thisThreadInput.z;
		if (thisThreadInput.z >= 3)
			paramData[thisThreadInput.y+2] += timeIncre;
	}

	if (bi < inputBufferSize)
	{
		rewriteSize[localB].x = 1;
		thisThread2Input = inLData[bi];
		rewriteSize[localB].y = thisThread2Input.z;
		if (thisThread2Input.z >= 3)
			paramData[thisThread2Input.y+2] += timeIncre;
	}


	bool aRC = false;
	bool bRC = false;
	[loop]
	for (int i = 0; i < ruleBufferSize; i+=7)
	{
		uint rulePointer = ruleOffSets[i];
		uint ruleProdPointer = ruleOffSets[i+1];
		uint ruleProdEnd = ruleOffSets[i+2];
		uint ruleSize = ruleOffSets[i+3];
		uint rulePSize = ruleOffSets[i+4];
		uint ruleRWSize = ruleOffSets[i+5];
		uint rulePRWSize = ruleOffSets[i+6];
		uint currRule = ruleData[rulePointer];
	//	if (buffCheck1){
			if ((currRule == thisThreadInput.x) & (!aRC))
			{
				bool r = checkRule(rulePointer, thisThreadInput.y, ruleProdPointer, paramData[thisThreadInput.y+2]);
				if (r == true)
				{
					rewriteSize[localA].x = ruleSize;
					rewriteSize[localA].y = rulePSize;
					threadRewritePos   = ruleProdPointer;
					threadRewriteSize  = ruleProdEnd;//threadRewritePos + ruleProdSize;
					rulePointers[ai].a = threadRewritePos;
					rulePointers[ai].b = threadRewriteSize;
					rulePointers[ai].c = ruleSize;
					rulePointers[ai].d = rulePSize;
					rulePointers[ai].e = ruleRWSize;
					rulePointers[ai].f = rulePRWSize;
					aRC = true;
					//break;
				}
			}
		//}
		//if(buffCheck2){
			if ((currRule == thisThread2Input.x) & (!bRC))
			{
				bool r = checkRule(rulePointer, thisThread2Input.y, ruleProdPointer, paramData[thisThread2Input.y+2]);
				if (r == true)
				{
					rewriteSize[localB].x = ruleSize;
					rewriteSize[localB].y = rulePSize;
					thread2RewritePos  = ruleProdPointer;
					thread2RewriteSize = ruleProdEnd;//thread2RewritePos + ruleProdSize;
					rulePointers[bi].a = thread2RewritePos;
					rulePointers[bi].b = thread2RewriteSize;
					rulePointers[bi].c = ruleSize;
					rulePointers[bi].d = rulePSize;
					rulePointers[bi].e = ruleRWSize;
					rulePointers[bi].f = rulePRWSize;
					bRC = true;
					//break;
				}
			}
		//}
	}

	GroupMemoryBarrierWithGroupSync();
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

			rewriteSize[bi2].x += rewriteSize[ai2].x;
			rewriteSize[bi2].y += rewriteSize[ai2].y; 
		}
		offset *= 2;
	}

	// clear the last element 
	if (GTid.x == 0) 
	{
		sums[Gid.x].x = rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)].x;
		sums[Gid.x].y = rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)].y;
		rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)].x = 0;
		//if (Gid.x == 0)
			rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)].y = tableBufferSize;//0;
		//else
		//	rewriteSize[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)].y = 0;
		rewriteFinalSize = 0;
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
 
			//float?
			uint l = rewriteSize[aii].x;
			rewriteSize[aii].x = rewriteSize[bii].x;
			rewriteSize[bii].x += l;

			uint p = rewriteSize[aii].y;
			rewriteSize[aii].y = rewriteSize[bii].y;
			rewriteSize[bii].y += p;
		}
	}
	//syn
	GroupMemoryBarrierWithGroupSync();
	dataSize[ai].x = rewriteSize[localA].x;
	dataSize[bi].x = rewriteSize[localB].x;
	dataSize[ai].y = rewriteSize[localA].y;
	dataSize[bi].y = rewriteSize[localB].y;
	
	//dataSize[ai].x = 0;
	//dataSize[bi].x = 0;
	//dataSize[ai].y = 0;
	//dataSize[bi].y = 0;

	rewriteMax[GTid.x].x = 0;
	rewriteMax[GTid.x].y = 0;

	if (inputBufferSize < GROUPDIMX)
	{
		uint rwt = 0;
		uint rwp = 0;
		///*
		if (ai < inputBufferSize)//(buffCheck1)
		{
			uint rSize = rewriteSize[localA].x;
			uint paramPtr = rewriteSize[localA].y;
			if (threadRewritePos > 0)
			{
				[loop]
				for (threadRewritePos; threadRewritePos<threadRewriteSize; threadRewritePos)
				{
					uint3 test = setLData(threadRewritePos, threadRewriteSize, thisThreadInput.y, paramPtr);
					outputLData[rSize] = test;
					rSize++;
				}
				rwt += rulePointers[ai].e;
				rwp += rulePointers[ai].f;
				if (rSize > rewriteFinalSize)
					rewriteFinalSize = rSize;
			}
			else
			{
				rwt++;
				rwp += thisThreadInput.z;
				outputLData[rSize] = thisThreadInput;
				//outputLData[rSize] = outputLData[ai];
				outputLData[rSize].y = paramPtr;
				uint pSize = thisThreadInput.y+thisThreadInput.z;
				[loop]
				for(uint i=thisThreadInput.y;i<pSize;i++)
				{
					paramDataRW[paramPtr] = paramData[i];
					paramPtr++;
				}
				if (rSize+1 > rewriteFinalSize)
					rewriteFinalSize = rSize+1;
			}
		}
		if (bi < inputBufferSize)//(buffCheck2)
		{
			uint rSize = rewriteSize[localB].x;
			uint paramPtr = rewriteSize[localB].y;
			if (thread2RewritePos > 0)
			{
				[loop]
				for (thread2RewritePos; thread2RewritePos<thread2RewriteSize; thread2RewritePos)
				{   
					uint3 test = setLData(thread2RewritePos, thread2RewriteSize, thisThread2Input.y, paramPtr);
					outputLData[rSize] = test;
					rSize++;
				}
				rwt += rulePointers[bi].e;
				rwp += rulePointers[bi].f;
			    if (rSize > rewriteFinalSize)
					rewriteFinalSize = rSize;
			}
			else
			{
				rwt++;
				rwp += thisThread2Input.z;
				outputLData[rSize] = thisThread2Input;
				//outputLData[rSize] = outputLData[bi];
				outputLData[rSize].y = paramPtr;
				uint pSize = thisThread2Input.y+thisThread2Input.z;
				[loop]
				for(uint i=thisThread2Input.y;i<pSize;i++)
				{
					paramDataRW[paramPtr] = paramData[i];
					paramPtr++;
				}
				if (rSize+1 > rewriteFinalSize)
					rewriteFinalSize = rSize+1;
			}
		}
		//*/

		rewriteMax[GTid.x].x = rwt;
		rewriteMax[GTid.x].y = rwp;
		//dataSize[GTid.x].x = rwt;
		//dataSize[GTid.x].y = rwp;

		AllMemoryBarrierWithGroupSync();
		if(GTid.x == 0)
		{
			dataSize[0].x = rewriteFinalSize; 
			
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
	}
}