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

cbuffer buffSize
{
	uint inputBufferSize;
	uint b1;
	uint b2;
	uint b3;
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
RWStructuredBuffer<uint7>		sums				: register(u1);
RWStructuredBuffer<int4>		countVals			: register(u2);
RWStructuredBuffer<float4x4>	orientation			: register(u3);
RWStructuredBuffer<float4>		vertices			: register(u4);
RWStructuredBuffer<uint>		indices				: register(u5);

//groupshared uint preSum[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared uint totalVers[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared int	 indPos[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
groupshared float4x4 prefixOrientation[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];

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

float3 mulMatrixVec(float3 inputVec, float3x3 inputMat)
{
	float3 output;
	output[0] = inputVec.x * inputMat[0][0] + inputVec.y * inputMat[1][0] + inputVec.z * inputMat[2][0];
	output[1] = inputVec.x * inputMat[0][1] + inputVec.y * inputMat[1][1] + inputVec.z * inputMat[2][1];
	output[2] = inputVec.x * inputMat[0][2] + inputVec.y * inputMat[1][2] + inputVec.z * inputMat[2][2];
	return output;
}

void rotationCalc(int sPos, uint localMem)
{
	uint dcSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);

	vector<float, 4> pos	 = {0,0,0,0};
	vector<float, 3> left	 = {1.f,0,0};
	vector<float, 3> up		 = {0,1.f,0};
	vector<float, 3> forward = {0,0,1.f};
	//float3x3 localOrien = float3x3 (1,0,0,0,1,0,0,0,1);

	//int vers = 0;
	uint vers = 0;
	int inds = 0;
	int startPos = sPos;

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
					}
					break;
				case 94: // ^
					{
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
							//0,
							//Row 2
							forward.x*forward.y*(1-cosq)-forward.z*sinq,
							(forward.y*forward.y)+(1-(forward.y*forward.y))*cosq,
							forward.x*forward.z*(1-cosq)+forward.x*sinq,
							//0,
							//Row 3
							forward.x*forward.z*(1-cosq)+forward.y*sinq,
							forward.y*forward.z*(1-cosq)-forward.x*sinq,
							(forward.z*forward.z)+(1-(forward.z*forward.z))*cosq,
							//0,
							//Row4
							//0,0,0,1
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
							//0,
							//Row 2
							forward.x*forward.y*(1-cosq)-forward.z*sinq,
							(forward.y*forward.y)+(1-(forward.y*forward.y))*cosq,
							forward.x*forward.z*(1-cosq)+forward.x*sinq,
							//0,
							//Row 3
							forward.x*forward.z*(1-cosq)+forward.y*sinq,
							forward.y*forward.z*(1-cosq)-forward.x*sinq,
							(forward.z*forward.z)+(1-(forward.z*forward.z))*cosq,
							//0,
							//Row4
							//0,0,0,1
						};
						up		= mulMatrixVec(up, m);
						forward = mulMatrixVec(forward, m);
						left	= mulMatrixVec(left, m);
					}
					break;
				case 36: // $
					{
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
					}
					break;
				case 76: // L
					/*
					leaves->AddLeaf(XMFLOAT3(currentRun.vector.x,currentRun.vector.y,currentRun.vector.z),
										currentRun.pitchV,
										currentRun.yawV,
										currentRun.rollV);
					*/
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
							inds = 0;
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
	//prefixOrientation[localMem] = float3x3(forward.x,forward.y,forward.z,up.x,up.y,up.z,left.x,left.y,left.z);
	prefixOrientation[localMem] = float4x4(left.x,left.y,left.z,0,up.x,up.y,up.z,0,forward.x,forward.y,forward.z,0,pos.x,pos.y,pos.z,1);
	totalVers[localMem] = vers;
	indPos[localMem] = inds;
}

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{			
	//uint cSize;
	//uint blank;
	//countVals.GetDimensions(cSize,blank);

	//uint7 s = sums[Gid.x];
	uint groupOffset = Gid.x*(GROUPDIMX*PARSECHUNK);
	uint ai = (GTid.x*PARSECHUNK) + groupOffset;
	uint bi = ai + (THREADS*PARSECHUNK);
	//uint bankOffsetA = CONFLICT_FREE_OFFSET(GTid.x); 
	//uint bankOffsetB = CONFLICT_FREE_OFFSET(GTid.x + THREADS);
	uint localA = GTid.x + CONFLICT_FREE_OFFSET(GTid.x)/*bankOffsetA*/;
	uint localB = (GTid.x + THREADS) + CONFLICT_FREE_OFFSET(GTid.x + THREADS)/*bankOffsetB*/;
	bool aB = false;
	bool bB = false;
	uint end = GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1);

	totalVers[localA] = 0;
	totalVers[localB] = 0;
	indPos[localA] = 0;
	indPos[localB] = 0;
	float4x4 null = float4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	prefixOrientation[localA] = null;
	prefixOrientation[localB] = null;
	if(DTid.x == 0)
	{
		orientation[0/*cSize*/] = float4x4(1,ai,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	}
	GroupMemoryBarrierWithGroupSync();

	///*
	//uint gP = s.q + s.u;
	if (ai < inputBufferSize)
	{
		//countsA = countVals[Gid.x];
		rotationCalc( ai, localA );
		aB = true;
	}
	
	if (bi < inputBufferSize)
	{
		//countsB = countVals[bi];
		rotationCalc( bi, localB );
		bB = true;
	}

	
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
		}
		offset *= 2;
	}

	// clear the last element 
	if (GTid.x == 0) 
	{
		//orientation[cSize + Gid.x] = prefixOrientation[end];
		//orientation[cSize + Gid.x][0][3] = totalVers[end];
		//orientation[((inputBufferSize/PARSECHUNK)+1) + (numBr/2) + Gid.x][3][2] = numBr;
		//orientation[((inputBufferSize/PARSECHUNK)+1) + (numBr/2) + Gid.x][3][0] = ((inputBufferSize/PARSECHUNK)+1);
		prefixOrientation[end] = null;
		totalVers[end] = 0;
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
		}
	}

	GroupMemoryBarrierWithGroupSync();
	if (aB==true && GTid.x>0)
	{

		orientation[ai/PARSECHUNK]	= prefixOrientation[localA];
		orientation[ai/PARSECHUNK][1][3] = indPos[localA];
		orientation[ai/PARSECHUNK][0][3] = totalVers[localA];//orientation[ai][3][1];
		//orientation[ai][1][2] = totalVers[localA];
		//countVals[ai] = countsA;
	}
	if (bB==true && GTid.x>0)
	{
		orientation[bi/PARSECHUNK] = prefixOrientation[localB];
		orientation[bi/PARSECHUNK][1][3] = indPos[localB];
		orientation[bi/PARSECHUNK][0][3] = totalVers[localB];//orientation[bi][3][1];
		//orientation[bi][1][2] = totalVers[localB];
		//countVals[bi] = countsB;
	}

	//*/
}