//--------------------------------------------------------------------------------------
// File: TreeCompute.hlsl
//
// Here we have a compute shader designed to take a set of points represeting the trunk and branches
// of a tree and calculates the points for the body of the tree 
//--------------------------------------------------------------------------------------

struct vertLayout
{
    float3 pos;
	float3 norm;
	float2 tex0;
};

cbuffer buffSize{
	uint vertSize;
	uint indSize;
	uint blank;
	uint pad;
}

#define SIDES 6

StructuredBuffer<float4> vertices	: register(t0);
StructuredBuffer<uint> indices		: register(t1);
StructuredBuffer<float> texCoord	: register(t2);
RWStructuredBuffer<vertLayout> vers : register(u0);
RWStructuredBuffer<uint> inds		: register(u1);

void buildPoint(uint nextInd, uint lastInd)
{
	//Build tree points
	//vers[0].pos.x = nextInd;
	//vers[0].pos.y = lastInd;
	float width = vertices[nextInd].w;
	float3 trunkVerts[SIDES];
	float angle = (360.f/SIDES)*(3.14159f/180);
	float angleI = angle;
	[loop]
	for (uint i=0; i<SIDES;i++)
	{
		float x = width*cos(angleI);
		float y = width*sin(angleI);
		trunkVerts[i] = float3(x,0,y);
		angleI = angleI + angle;
	}

	//Find branch angle
	vector<float, 3> v = {0.f,1.f,0.f};
	vector<float, 3> V = {vertices[nextInd].x-vertices[lastInd].x,vertices[nextInd].y-vertices[lastInd].y,vertices[nextInd].z-vertices[lastInd].z};
	vector<float, 3> axis = cross(v,V);
	float vecAngle = 0;
	float v1 = length(axis);
	if (v1>0){
		V = normalize(V);
		vecAngle = dot(v,V);
		vecAngle = acos(vecAngle);
		// Build rotation matrix
		float cosq = cos(vecAngle);//0.785397589;//
		float sinq = sin(vecAngle);
		axis = normalize(axis);
		float4x4 m = 
		{				//Row 1
						(axis.x*axis.x)+(1-(axis.x*axis.x))*cosq,
						axis.x*axis.y*(1-cosq)+axis.z*sinq,
						axis.x*axis.z*(1-cosq)-axis.y*sinq,
						0,
						//Row 2
						axis.x*axis.y*(1-cosq)-axis.z*sinq,
						(axis.y*axis.y)+(1-(axis.y*axis.y))*cosq,
						axis.x*axis.z*(1-cosq)+axis.x*sinq,
						0,
						//Row 3
						axis.x*axis.z*(1-cosq)+axis.y*sinq,
						axis.y*axis.z*(1-cosq)-axis.x*sinq,
						(axis.z*axis.z)+(1-(axis.z*axis.z))*cosq,
						0,
						//Row4
						0,0,0,1
		};

		// Apply matrix to verts
		[loop]
		for (i = 0;i<SIDES;i++){
			//v =  XMVector3Transform(trunkVerts[i],m);
			float4 r;
			r[0] = trunkVerts[i].x * m[0][0] + trunkVerts[i].y * m[1][0] + trunkVerts[i].z * m[2][0] + 1 * m[3][0];
			r[1] = trunkVerts[i].x * m[0][1] + trunkVerts[i].y * m[1][1] + trunkVerts[i].z * m[2][1] + 1 * m[3][1];
			r[2] = trunkVerts[i].x * m[0][2] + trunkVerts[i].y * m[1][2] + trunkVerts[i].z * m[2][2] + 1 * m[3][2];
			r[3] = trunkVerts[i].x * m[0][3] + trunkVerts[i].y * m[1][3] + trunkVerts[i].z * m[2][3] + 1 * m[3][3];
			r.x = r.x/r.w;
			r.y = r.y/r.w;
			r.z = r.z/r.w;
			float3 r2 =  {r.x,r.y,r.z};
			trunkVerts[i] = r2;
		}
	}

	//Position tree points at both ends of the section
	uint outcount = (nextInd*SIDES);
	[loop]
	for (i = 0;i<SIDES;i++){
		vers[outcount].pos.x = vertices[nextInd].x + trunkVerts[i].x; 
		vers[outcount].pos.y = vertices[nextInd].y + trunkVerts[i].y; 
		vers[outcount].pos.z = vertices[nextInd].z + trunkVerts[i].z;
		vers[outcount].norm  = trunkVerts[i];
		vers[outcount].tex0 = float2(texCoord[i],nextInd);
		outcount++;
	}
}

//*
void buildInds(uint nextInd, uint lastInd)
{
	uint upperVertCount = nextInd*6;
	uint lowerVertCount = lastInd*6;
	uint lastSecL = lowerVertCount;
	uint lastSecU = upperVertCount;
	uint b = (nextInd-1) * (SIDES*6);
	uint size = b+((SIDES-1)*6);
	/*
	if(nextInd == 3)
	{
		inds[72] = 99;
	}
	*/
	[loop]
	for (;b<size;b=b+6)
	{
		inds[b]   = lowerVertCount;
		inds[b+1] = upperVertCount;
		inds[b+2] = lowerVertCount+1;
		inds[b+3] = upperVertCount;
		inds[b+4] = upperVertCount+1;
		inds[b+5] = lowerVertCount+1;
		upperVertCount++;
		lowerVertCount++;
	}
	inds[b]   = lowerVertCount;
	inds[b+1] = upperVertCount;
	inds[b+2] = lastSecL;
	inds[b+3] = upperVertCount;
	inds[b+4] = lastSecU;//(section*SIDES)+SIDES;
	inds[b+5] = lastSecL;
}
//*/

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID/*, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID*/ )
{
	if (DTid.x < vertSize)
	{
		uint next;
		uint last;
		uint preCheck;
		if(DTid.x != 0)
		{
			next = indices[(DTid.x*2)+1];
			last = indices[DTid.x*2];
			preCheck = indices[(DTid.x*2)-1];
		}
		else
		{
			next = 1;
			last = 0;
			preCheck = 0;
		}

		buildPoint(next, last);
		buildInds(next, last);
		if (DTid.x == 0 || preCheck != last)
		{
			buildPoint(last, last);		
		}
	}
}