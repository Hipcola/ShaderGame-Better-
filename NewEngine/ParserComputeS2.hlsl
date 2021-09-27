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

void build(uint sPos)
{
	uint dcSize;
	uint cSize;
	uint blank;
	drawChars.GetDimensions(dcSize,blank);
	countVals.GetDimensions(cSize,blank);
	int startPos = sPos;

	//float4x4 o;
	vector<float, 4> pos;//	 = {0,0,0,0};
	vector<float, 3> left;//	 = {1.f,0,0};
	vector<float, 3> up;//		 = {0,1.f,0};
	vector<float, 3> forward;// = {0,0,1.f};
	uint vers = 0;
	int inds = 0;
	float4x4 o = float4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	if (sPos != 0)
	{
		uint oP = sPos/PARSECHUNK;
		o = orientation[oP];
		inds = o[1][3];
		vers = o[0][3];
		//debug comment out
		//orientation[10+sumOS.v+tPos] = float4x4(left.x,left.y,left.z,vers,up.x,up.y,up.z,tPos,forward.x,forward.y,forward.z,sumOS.u,pos.x,pos.y,pos.z,startPos);
	}
	else{
		vertices[0] = float4(0,0,0,0.2);
	}
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
							pos = float4(pos.x+p.x, pos.y+p.y, pos.z+p.z ,0.2);
							//pos = float4(sumOS.v, pos.y+p.y, sumOS.w,gi);
							/*
							if (vers == 6)
							{
								orientation[14] = float4x4(left.x,left.y,left.z,i,up.x,up.y,up.z,sumOS.q,forward.x,forward.y,forward.z,sumOS.v,pos.x,pos.y,pos.z,parsePos);
							}
							*/
							vertices[vers+1] = pos;
							indices[(vers*2)] = vers;//vers+inds;
							indices[(vers*2)+1] = vers+1;

							vers++;
							inds = 0;
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
	/*
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
	*/

	//uint7 s = sums[Gid.x + dData];
	uint groupOffset = Gid.x*(GROUPDIMX*PARSECHUNK);
	uint ai = GTid.x + groupOffset;
	uint bi = ai + (THREADS*PARSECHUNK);
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
	if ( ai < inputBufferSize )
	{
		countsA = countVals[ai];
		build( ai );
		//countVals[ai] = int4(0,0,0,0);
	}
	
	if ( bi < inputBufferSize )
	{
		countsB = countVals[bi];
		build( bi );
		//countVals[bi] = int4(0,0,0,0);
	}
	
}