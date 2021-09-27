//--------------------------------------------------------------------------------------
// File: ParserCompute4.hlsl
//
// L-System parser
//--------------------------------------------------------------------------------------

#define NUM_BANKS 16 
#define GROUPDIMX 32//256
//#define VERGROUPMAX 256
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

//groupshared float4	 threadPos[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];
//groupshared float4	 totalPos[VERGROUPMAX+(VERGROUPMAX/NUM_BANKS)];
//groupshared float3x3 prefixOrientation[GROUPDIMX+(GROUPDIMX/NUM_BANKS)];

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

void build(int4 counts, uint tPos, uint7 sumOS, uint gi)
{
	uint dcSize;
	uint cSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);
	countVals.GetDimensions(cSize,blank);
	uint startPos = counts.x;

	//float4x4 o;
	vector<float, 4> pos	 = {0,0,0,0};
	vector<float, 3> left	 = {1.f,0,0};
	vector<float, 3> up		 = {0,1.f,0};
	vector<float, 3> forward = {0,0,1.f};
	uint vers = 0;
	uint inds = 0;
	float4x4 o = float4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	float4x4 br = orientation[cSize+sumOS.w];
	uint brI = br[1][3];
	uint brV = br[0][3];
	inds = brI;
	//float4x4 o = float4x4(0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,1);
	if (tPos != sumOS.v)
	{
		o = orientation[(sumOS.v+(tPos-sumOS.v))-1];
		inds += o[1][3];
		vers = o[0][3];
		//debug comment out
		//orientation[10+sumOS.v+tPos] = float4x4(left.x,left.y,left.z,vers,up.x,up.y,up.z,tPos,forward.x,forward.y,forward.z,sumOS.u,pos.x,pos.y,pos.z,startPos);
	}/*else{
		//WILL BRAKE FOR SUMMING OVER GROUPS
		vertices[0] = float4(0,0,0,0.2);
	}*/

	//if (sumOS.v == 5)
	//{
	//	indices[0] = inds;
	//}
	vers = vers + brV;

	br[1][3] = 0;
	br[0][3] = 0;
	o[1][3] = 0;
	o[0][3] = 0;
	o = mul(o,br);

	pos = float4(o[3][0],o[3][1],o[3][2],0);
	left	= o[0];
	up		= o[1];
	forward = o[2];

	//orientation[10+sumOS.v+tPos] = float4x4(left.x,left.y,left.z,vers,up.x,up.y,up.z,tPos,forward.x,forward.y,forward.z,sumOS.v,pos.x,pos.y,pos.z,startPos);

	[loop]
	for (uint i = 0; i<PARSECHUNK; i++)
	{
		int parsePos = startPos+i;
		[branch]
		if (parsePos < /**/inputBufferSize/*sumOS.w*/)
		{
			uint3 input = inLData[parsePos];
			[branch]
			switch(input.x)
			{
				case 91: // [
					{
						uint jumpTo = paramData[input.y];
						orientation[cSize+((jumpTo-1)/2)] = float4x4(left.x,left.y,left.z,vers,up.x,up.y,up.z,inds,forward.x,forward.y,forward.z,0,pos.x,pos.y,pos.z,1);

						uint7 bjt = brackets[jumpTo];
						uint7 bjb = brackets[jumpTo-1];
					    int verAdd = bjt.v - bjb.v;
						//orientation[cSize+((jumpTo-1)/2)] = float4x4(left.x,left.y,left.z,vers,up.x,up.y,up.z,inds,forward.x,forward.y,forward.z,0,pos.x,pos.y,pos.z,verAdd);
						//inds -= verAdd;
						vers += verAdd;
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
						//drawWidth = ldata.data[0];
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
							///*
							float3x3 m = 
							{	
								left, up, forward
							};
							float3 p = float3( 0, paramData[input.y], 0 );
							//float3 = float;
							p = mulMatrixVec(p, m);
							//float4 pt = float4(p.x,p.y,p.z,0); 
							//pos += pt;
							//totalPos[(verPos+versLocal)+CONFLICT_FREE_OFFSET(verPos+versLocal)] = float4(p.x,p.y,p.z,1);
							pos = float4(pos.x+p.x, pos.y+p.y, pos.z+p.z, 0.05);

							/*
							if ( dSize == 4 && vers == 9)
							{
								indices[5] = 99;
								indices[4] = i;
								indices[3] = gi;
								indices[2] = tPos;
								indices[1] = startPos;
								indices[0] = parsePos;//vers+inds;
							}*/
							/*
							if( dSize == 4 && currDepth == 1)
							{
								if(sumOS.v == 3)
								{
									vertices[vers+1] = pos;
									indices[(vers*2)] = inds;//vers+inds;
									indices[(vers*2)+1] = vers+1;
								}
							}
							else
							{
								*/
								vertices[vers+1] = pos;
								indices[(vers*2)] = inds;//vers+inds;
								indices[(vers*2)+1] = vers+1;
							//}

							vers++;
							inds = vers;
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
	//threadPos[localMem] = ;
	//prefixOrientation[localMem] = float3x3(forward.x,forward.y,forward.z,up.x,up.y,up.z,left.x,left.y,left.z);
}

[numthreads(THREADS, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	uint dData = depthSizes.Load((currDepth+1)*12);
	//uint3 nData;
	if (DTid.x == 0 && currDepth != dSize)
	{
		uint nData = depthSizes.Load((currDepth+2)*12);
		uint fData;
		if (currDepth == dSize-1)
			fData = (numBr/2)+1;
		else
			fData = depthSizes.Load((currDepth+3)*12);

		//uint3 lastD = depthSizes.Load(0);
		//depthSizes.Store3((currDepth+2)*12, uint3(lastD.x,dData.y,dData.z));
		depthSizes.Store3(0, uint3(fData-nData,1,1));
		//if(DTid.x == 0)
		//	vertices[0] = float4(nData.x,dData.x,0,0);
	}

	uint7 s = sums[Gid.x + dData];
	uint groupOffset = /*(s.u-Gid.x)*/0*GROUPDIMX;
	uint ai = GTid.x + groupOffset + s.v;
	uint bi = ai + THREADS;
	bool aB = false;
	bool bB = false;
	//uint end = VERGROUPMAX - 1 + CONFLICT_FREE_OFFSET(VERGROUPMAX - 1);
	//float4 endVert;

	int4 countsA;
	int4 countsB;
	//float3x3 null = float3x3(1,0,0,0,1,0,0,0,1);
	//prefixOrientation[localA] = null;
	//prefixOrientation[localB] = null;

	//TODO Build in sum reading and maybe bracket rot passing into the build function
	//uint gP = s.q + s.u;
	if ( ai < (s.v + s.q) )
	{
		countsA = countVals[ai];
		build( countsA, ai, s, GTid.x);
		//countVals[ai] = int4(0,0,0,0);
	}
	
	if ( bi < (s.v + s.q) )
	{
		//countsB = countVals[bi];
		//build( countsB, bi, s, GTid.x);
		//countVals[bi] = int4(0,0,0,0);
	}
	
	/*
	//reduction step
	uint offset = 1;
	[loop]
	for (int a = VERGROUPMAX>>1; a > 0; a >>= 1)
	{
		//syn
		GroupMemoryBarrierWithGroupSync();
		if (GTid.x < a) 
		{ 
			int ai2 = offset*(2*GTid.x+1)-1;
			int bi2 = offset*(2*GTid.x+2)-1;
 
			ai2 += CONFLICT_FREE_OFFSET(ai2); 
			bi2 += CONFLICT_FREE_OFFSET(bi2); 

			totalPos[bi2] += totalPos[ai2];
		}
		offset *= 2;
	}
	
	if (GTid.x == 0) 
	{
		endVert = totalPos[end]; 
		totalPos[end] = float4(0,0,0,0);
	} 

	[loop]
	for (int b = 1; b < VERGROUPMAX; b *= 2) // traverse down tree & build scan 
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

			float4 l = totalPos[aii];
			totalPos[aii] = totalPos[bii];
			totalPos[bii] += l;
		}
	}

	/*
	offset = 1;
	[loop]
	for (int a = GROUPDIMX>>1; a > 0; a >>= 1)
	{
		//syn
		GroupMemoryBarrierWithGroupSync();
		if (GTid.x < a) 
		{ 
			int ai2 = offset*(2*GTid.x+1)-1;
			int bi2 = offset*(2*GTid.x+2)-1;
 
			ai2 += CONFLICT_FREE_OFFSET(ai2); 
			bi2 += CONFLICT_FREE_OFFSET(bi2); 

			threadPos[bi2] += threadPos[ai2];
		}
		offset *= 2;
	}	
	
	if (GTid.x == 0) 
		threadPos[GROUPDIMX - 1 + CONFLICT_FREE_OFFSET(GROUPDIMX - 1)] = float4(0,0,0,0);

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

			float4 l = totalVers[aii];
			totalVers[aii] = totalVers[bii];
			totalVers[bii] += l;
		}
	}
	//*/
	/*
	[unroll(PARSECHUNK)]
	for(uint i = 0 ;; i++)
	{
		vertices[t1+i] = totalPos[t1+i];
		vertices[t2+i] = totalPos[t2+i];
		vertices[t1+i].x = GTid.x;
		vertices[t2+i].x = GTid.x;
		//vertices[vers] = totalPos[t2+i];
	}
	//*/
	/*
	if (aB==true)
		write( countsA, ai, t1 );
	if (bB==true)
		write( countsB, bi, t2 );
	//*/
}