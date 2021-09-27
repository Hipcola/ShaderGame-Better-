//--------------------------------------------------------------------------------------
// File: ParserCompute4.hlsl
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

StructuredBuffer<uint>			drawChars			: register(t0);
StructuredBuffer<uint3>			inLData				: register(t1);

RWStructuredBuffer<float>		paramData		    : register(u0);
RWStructuredBuffer<float4x4>	orientation			: register(u1);
RWStructuredBuffer<int4>		countVals			: register(u2);
RWStructuredBuffer<float4>		vertices			: register(u3);
RWStructuredBuffer<uint>		indices				: register(u4);
RWByteAddressBuffer/*<uint3>*/	depthSizes			: register(u5);
RWStructuredBuffer<uint7>		brackets			: register(u6);
RWStructuredBuffer<uint7>		sums				: register(u7);

groupshared uint		totalVers[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared uint		indPos[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared uint		indSto[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared uint		indOff[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared float4x4	prefixOrientation[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
//groupshared float4   posLocal[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
//x = verts
//y = inds
//z = bracketsVal
//w = bracketsValScanned

float3 mulMatrixVec(float3 inputVec, float3x3 inputMat)
{
	float3 output;
	output[0] = inputVec.x * inputMat[0][0] + inputVec.y * inputMat[1][0] + inputVec.z * inputMat[2][0];
	output[1] = inputVec.x * inputMat[0][1] + inputVec.y * inputMat[1][1] + inputVec.z * inputMat[2][1];
	output[2] = inputVec.x * inputMat[0][2] + inputVec.y * inputMat[1][2] + inputVec.z * inputMat[2][2];
	return output;
}

void rotationCalc(int4 counts, uint localMem)
{
	uint dcSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);
	
	vector<float, 4> pos	 = {0,0,0,0};
	vector<float, 3> left	 = {1.f,0,0};
	vector<float, 3> up		 = {0,1.f,0};
	vector<float, 3> forward = {0,0,1.f};
	float width = 0.0f;
	//float3x3 localOrien = float3x3 (1,0,0,0,1,0,0,0,1);

	//int vers = 0;
	uint vers = 0;
	uint inds = 0;
	uint indStored = 0;
	int startPos = counts.x;

	[loop]
	for (uint i = 0; i<PARSECHUNK; i++)
	{
		int parsePos = startPos+i;
		[branch]
		if (parsePos < /*sumOS.w*/ inputBufferSize)
		{
			uint3 input = inLData[parsePos];
			[branch]
			switch(input.x)
			{
				case 91: // [
					{
						uint jumpTo = paramData[input.y];
						uint7 bjt = brackets[jumpTo];
						uint7 bjb = brackets[jumpTo-1];

						int verAdd = bjt.v - bjb.v;
						vers += verAdd;
						indStored += verAdd;
						startPos = bjt.x-i;
					}
					break;
				case 93: // ]
					{
						i = PARSECHUNK;
					}
					break;
				case 43: // +
					{
						float vecAngle = paramData[input.y];
						float cosq = cos(vecAngle);
						float sinq = sin(vecAngle);
						float3x3 m = 
						{	
							//Row 1
							(up.x*up.x)+(1-(up.x*up.x))*cosq,
							up.x*up.y*(1-cosq)+up.z*sinq,
							up.x*up.z*(1-cosq)-up.y*sinq,
							//Row 2
							up.x*up.y*(1-cosq)-up.z*sinq,
							(up.y*up.y)+(1-(up.y*up.y))*cosq,
							up.x*up.z*(1-cosq)+up.x*sinq,
							//Row 3
							up.x*up.z*(1-cosq)+up.y*sinq,
							up.y*up.z*(1-cosq)-up.x*sinq,
							(up.z*up.z)+(1-(up.z*up.z))*cosq,
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 45: // -
					{
						float vecAngle = -paramData[input.y];
						float cosq = cos(vecAngle);
						float sinq = sin(vecAngle);
						float3x3 m = 
						{	
							//Row 1
							(up.x*up.x)+(1-(up.x*up.x))*cosq,
							up.x*up.y*(1-cosq)+up.z*sinq,
							up.x*up.z*(1-cosq)-up.y*sinq,
							//0,
							//Row 2
							up.x*up.y*(1-cosq)-up.z*sinq,
							(up.y*up.y)+(1-(up.y*up.y))*cosq,
							up.x*up.z*(1-cosq)+up.x*sinq,
							//0,
							//Row 3
							up.x*up.z*(1-cosq)+up.y*sinq,
							up.y*up.z*(1-cosq)-up.x*sinq,
							(up.z*up.z)+(1-(up.z*up.z))*cosq,
							//0,
							//Row4
							//0,0,0,1
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 38: // &
					{
						float vecAngle = paramData[input.y];
						float cosq = cos(vecAngle);
						float sinq = sin(vecAngle);
						float3x3 m = 
						{	
							//Row 1
							(left.x*left.x)+(1-(left.x*left.x))*cosq,
							left.x*left.y*(1-cosq)+left.z*sinq,
							left.x*left.z*(1-cosq)-left.y*sinq,
							//Row 2
							left.x*left.y*(1-cosq)-left.z*sinq,
							(left.y*left.y)+(1-(left.y*left.y))*cosq,
							left.x*left.z*(1-cosq)+left.x*sinq,
							//Row 3
							left.x*left.z*(1-cosq)+left.y*sinq,
							left.y*left.z*(1-cosq)-left.x*sinq,
							(left.z*left.z)+(1-(left.z*left.z))*cosq,
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 94: // ^
					{
						float vecAngle = -paramData[input.y];
						float cosq = cos(vecAngle);
						float sinq = sin(vecAngle);
						float3x3 m = 
						{	
							//Row 1
							(left.x*left.x)+(1-(left.x*left.x))*cosq,
							left.x*left.y*(1-cosq)+left.z*sinq,
							left.x*left.z*(1-cosq)-left.y*sinq,
							//Row 2
							left.x*left.y*(1-cosq)-left.z*sinq,
							(left.y*left.y)+(1-(left.y*left.y))*cosq,
							left.x*left.z*(1-cosq)+left.x*sinq,
							//Row 3
							left.x*left.z*(1-cosq)+left.y*sinq,
							left.y*left.z*(1-cosq)-left.x*sinq,
							(left.z*left.z)+(1-(left.z*left.z))*cosq,
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 47: // /
					{
						float vecAngle = paramData[input.y];
						float cosq = cos(vecAngle);
						float sinq = sin(vecAngle);
						float3x3 m = 
						{	
							//Row 1
							(forward.x*forward.x)+(1-(forward.x*forward.x))*cosq,
							forward.x*forward.y*(1-cosq)+forward.z*sinq,
							forward.x*forward.z*(1-cosq)-forward.y*sinq,
							//Row 2
							forward.x*forward.y*(1-cosq)-forward.z*sinq,
							(forward.y*forward.y)+(1-(forward.y*forward.y))*cosq,
							forward.x*forward.z*(1-cosq)+forward.x*sinq,
							//Row 3
							forward.x*forward.z*(1-cosq)+forward.y*sinq,
							forward.y*forward.z*(1-cosq)-forward.x*sinq,
							(forward.z*forward.z)+(1-(forward.z*forward.z))*cosq
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 92: // '\'
					{
						float vecAngle = -paramData[input.y];
						float cosq = cos(vecAngle);
						float sinq = sin(vecAngle);
						float3x3 m = 
						{	
							//Row 1
							(forward.x*forward.x)+(1-(forward.x*forward.x))*cosq,
							forward.x*forward.y*(1-cosq)+forward.z*sinq,
							forward.x*forward.z*(1-cosq)-forward.y*sinq,
							//Row 2
							forward.x*forward.y*(1-cosq)-forward.z*sinq,
							(forward.y*forward.y)+(1-(forward.y*forward.y))*cosq,
							forward.x*forward.z*(1-cosq)+forward.x*sinq,
							//Row 3
							forward.x*forward.z*(1-cosq)+forward.y*sinq,
							forward.y*forward.z*(1-cosq)-forward.x*sinq,
							(forward.z*forward.z)+(1-(forward.z*forward.z))*cosq
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 36: // $
					{
						left = cross(vector(0.f,-1.f,0.f,1.f),left);
						forward = cross(up,left);
						/*forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						left = XMVector3Cross(XMVectorSet(0.f,-1.f,0.f,1.f),up);
						forward = XMVector3Cross(up,left);
						XMStoreFloat4(&currentRun.rollV,forward);
						XMStoreFloat4(&currentRun.yawV, up);
						XMStoreFloat4(&currentRun.pitchV,left);//*/
					}
					break;
				case 33: // ! 
					{
						width = paramData[input.y];
					}
					break;
				case 76: // L
					break;
				default:
					{
						///*
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
							inds = vers;
							//inds = indStored;//vers;
							indStored = 0;
							float3x3 m = 
							{	
								left, up, forward
							};
							float3 p = float3( 0, paramData[input.y], 0 );
							//if (localMem == 0 && vers == 1)
							//{
							//	orientation[localMem][0][3] = parsePos;// paramData[8];
							//}
							p = mulMatrixVec(p, m);
							pos = float4(pos.x+p.x,pos.y+p.y,pos.z+p.z,0);
						}
						//*/
					}
				break;
				;
			}
		}else{
			i = PARSECHUNK;
		}
	}
	uint e = localMem + CONFLICT_FREE_OFFSET(localMem);
	//prefixOrientation[localMem] = float3x3(forward.x,forward.y,forward.z,up.x,up.y,up.z,left.x,left.y,left.z);
	prefixOrientation[e] = float4x4(left.x,left.y,left.z,0,up.x,up.y,up.z,0,forward.x,forward.y,forward.z,0,pos.x,pos.y,pos.z,1);
	totalVers[e] = vers;
	indPos[e] = inds;
	indSto[e] = indStored;

	//Bool?
	if (indStored > 0)
		indOff[e] = 1;
	else
		indOff[e] = 0;
}

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{			
	uint cSize;
	uint blank;
	countVals.GetDimensions(cSize,blank);
	//Very bad. Fix
	if(currDepth == 0 && DTid.x == 0)
	{
		vertices[0] = float4(0,0,0,0.05);
		orientation[cSize] = float4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	}
	GroupMemoryBarrierWithGroupSync();

	uint dData = depthSizes.Load((currDepth+1)*12);
	/*
	uint3 nData;
	if (currDepth != dSize)
		nData = uint3(cSize,0,0);
	else
	{
		nData = depthSizes.Load((currDepth+2)*12);
		//if(DTid.x == 0)
		//vertices[0] = float4(nData.x,dData.x,0,0);
	}
	*/

	uint7 s = sums[Gid.x + dData];
	uint groupOffset = /*(s.u-Gid.x)*/0*GROUPDIMX;
	uint ai = GTid.x + groupOffset + s.v;
	uint bi = ai + THREADS;
	//uint bankOffsetA = CONFLICT_FREE_OFFSET(GTid.x); 
	//uint bankOffsetB = CONFLICT_FREE_OFFSET(GTid.x + THREADS);
	uint localA = GTid.x + CONFLICT_FREE_OFFSET(GTid.x)/*bankOffsetA*/;
	uint localB = (GTid.x + THREADS) + CONFLICT_FREE_OFFSET(GTid.x + THREADS)/*bankOffsetB*/;
	bool aB = false;
	bool bB = false;
	uint end = GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1);

	int4 countsA;
	int4 countsB;
	totalVers[localA] = 0;
	totalVers[localB] = 0;
	indPos[localA] = 0;
	indPos[localB] = 0;
	indSto[localA] = 0;
	indSto[localB] = 0;
	indOff[localA] = 0;
	indOff[localB] = 0;
	float4x4 null = float4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	prefixOrientation[localA] = null;
	prefixOrientation[localB] = null;

	uint gP = s.v + s.q;
	if (ai < gP)
	{
		countsA = countVals[ai];
		rotationCalc( countsA, GTid.x );
		aB = true;
	}
	
	if (bi < gP)
	{
		countsB = countVals[bi];
		rotationCalc( countsB, GTid.x + THREADS );
		bB = true;
	}

	/*
	GroupMemoryBarrierWithGroupSync();
	if (GTid.x == 0 && dSize == 5)
	{
		orientation[3][0][0] = indPos[0];
		orientation[3][1][0] = indPos[1];
		orientation[3][2][0] = indPos[2];
		orientation[3][3][0] = indPos[3];
		orientation[3][0][1] = totalVers[0];
		orientation[3][1][1] = totalVers[1];
		orientation[3][2][1] = totalVers[2];
		orientation[3][3][1] = totalVers[3];
		orientation[3][0][2] = indSto[0];
		orientation[3][1][2] = indSto[1];
		orientation[3][2][2] = indSto[2];
		orientation[3][3][2] = indSto[3];
		orientation[3][0][3] = indOff[0];
		orientation[3][1][3] = indOff[1];
		orientation[3][2][3] = indOff[2];
		orientation[3][3][3] = indOff[3];
	}
	//*/

	///*
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

			totalVers[bi2] += totalVers[ai2];
			prefixOrientation[bi2] = mul(prefixOrientation[bi2],prefixOrientation[ai2]);
			//if (indPos[ai2] > indPos[bi2])
			//	indPos[bi2] = indPos[ai2];
			indPos[bi2] += indPos[ai2];
			indSto[bi2] += indSto[ai2];
			indOff[bi2] += indOff[ai2];
		}
		offset *= 2;
	}

	// clear the last element 
	if (GTid.x == 0) 
	{
		//groupOrientation[Gid.x] = null;//prefixOrientation[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)];
		//float3x3 sOrien = prefixOrientation[end];
		/*float4x4 o = float4x4( sOrien[0][0], sOrien[0][1], sOrien[0][2], totalVers[end],
							   sOrien[1][0], sOrien[1][1], sOrien[1][2], 0,
							   sOrien[2][0], sOrien[2][1], sOrien[2][2], 0,
							   0,0,0,);
		///*/

		orientation[cSize + (numBr/2) + (Gid.x+1)] = prefixOrientation[end];
		orientation[cSize + (numBr/2) + (Gid.x+1)][0][3] = totalVers[end];
		//orientation[((inputBufferSize/PARSECHUNK)+1) + (numBr/2) + Gid.x][3][2] = numBr;
		//orientation[((inputBufferSize/PARSECHUNK)+1) + (numBr/2) + Gid.x][3][0] = ((inputBufferSize/PARSECHUNK)+1);
		prefixOrientation[end] = null;
		totalVers[end] = 0;
		indPos[end] = 0;
		indSto[end] = 0;
		indOff[end] = 0;
	} 
	//*
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

			float4x4 or = prefixOrientation[aii];
			prefixOrientation[aii] = prefixOrientation[bii];
			prefixOrientation[bii] = mul(or,prefixOrientation[bii]);

			uint v = totalVers[aii];
			totalVers[aii] = totalVers[bii];
			totalVers[bii] += v;

			uint indSwap = indPos[aii];
			indPos[aii] = indPos[bii];
			indPos[bii] += indSwap;

			uint iSs = indSto[aii];
			indSto[aii] = indSto[bii];
			indSto[bii] += iSs;

			uint iSo = indOff[aii];
			indOff[aii] = indOff[bii];
			indOff[bii] += iSo;
		}
	}
	//*/

	GroupMemoryBarrierWithGroupSync();

	/*
	if (GTid.x == 0 && dSize == 5)
	{
		orientation[4][0][0] = indPos[0];
		orientation[4][1][0] = indPos[1];
		orientation[4][2][0] = indPos[2];
		orientation[4][3][0] = indPos[3];
		orientation[4][0][1] = totalVers[0];
		orientation[4][1][1] = totalVers[1];
		orientation[4][2][1] = totalVers[2];
		orientation[4][3][1] = totalVers[3];
		orientation[4][0][2] = indSto[0];
		orientation[4][1][2] = indSto[1];
		orientation[4][2][2] = indSto[2];
		orientation[4][3][2] = indSto[3];
		orientation[4][0][3] = indOff[0];
		orientation[4][1][3] = indOff[1];
		orientation[4][2][3] = indOff[2];
		orientation[4][3][3] = indOff[3];
	}
	//*/

	if (aB==true && GTid.x>0)
	{
		/*float3x3 sOrien = prefixOrientation[localA];
		float4x4 o = float4x4( sOrien[0][0], sOrien[0][1], sOrien[0][2], totalVers[localA],
							   sOrien[1][0], sOrien[1][1], sOrien[1][2], 0,
							   sOrien[2][0], sOrien[2][1], sOrien[2][2], 0,
							   0,0,0,0 );*/
		orientation[ai-1]	= prefixOrientation[localA];
		//Is this really happening right now?
		uint itrhrn = GTid.x-1 +  CONFLICT_FREE_OFFSET(indOff[GTid.x-1]);
		uint itA = indOff[itrhrn] + CONFLICT_FREE_OFFSET(indOff[itrhrn]);
		orientation[ai-1][1][3] = indPos[localA/*GTid.x-1*/] + indSto[itA];
		orientation[ai-1][0][3] = totalVers[localA];
		/*
		if (indPos[localA] == 1 && dSize == 4)
		{
			orientation[ai-1][0][0] = indPos[0];
			orientation[ai-1][1][0] = indPos[1];
			orientation[ai-1][2][0] = indPos[2];
			orientation[ai-1][3][0] = indPos[3];
		}
		*/
		//orientation[ai-1][2][3] = indPos[GTid.x+1];
		//orientation[ai-1][0][3] = totalVers[localA];//orientation[ai][3][1];
		//orientation[ai][1][2] = totalVers[localA];
		//countVals[ai] = countsA;
	}
	if (bB==true && GTid.x>0)
	{
		/*float3x3 sOrien = prefixOrientation[localB];
		float4x4 o = float4x4( sOrien[0][0], sOrien[0][1], sOrien[0][2], totalVers[localB],
							   sOrien[1][0], sOrien[1][1], sOrien[1][2], 0,
							   sOrien[2][0], sOrien[2][1], sOrien[2][2], 0,
							   0,0,0,0 );*/
		orientation[bi-1] = prefixOrientation[localB];
		orientation[bi-1][1][3] = indPos[localB];
		orientation[bi-1][0][3] = totalVers[localB];//orientation[bi][3][1];
		//orientation[bi][1][2] = totalVers[localB];
		//countVals[bi] = countsB;
	}
	//*/
	//GroupMemoryBarrierWithGroupSync();

	/*
	GroupMemoryBarrierWithGroupSync();
	if (parseStartA < inputBufferSize)
		 countVals[ai].x = prefixSum[localA];

	if (parseStartB < inputBufferSize)
		 countVals[bi].x = prefixSum[localB];
	*/

}