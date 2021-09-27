//--------------------------------------------------------------------------------------
// File: TreeCompute.hlsl
//
// Here we have a compute shader designed to render the trunk and branches
// for a trees skeleton
//--------------------------------------------------------------------------------------

struct vertLayout
{
    float3 pos;
	float3 norm;
	float2 tex0;
};

StructuredBuffer<float4> vertices;
StructuredBuffer<uint> indices;
RWStructuredBuffer<vertLayout> vers;

#define SIDES 6
//#define size pass in?


[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	//*
	//First we set up the texture data which is the same for all verts
	float texcoord[SIDES];
	float texCount = 1.0f/(SIDES-1);
	float count = 0;
	[loop]
	for (int a = 0;a<SIDES;a++){
		texcoord[a] = count;
		count += texCount;
	}
	//Get position data
	uint next = indices[(DTid.x*2)-1];
	//next = (next/65536)-1;
	uint last;
	if(DTid.x != 0)
	{
		last = indices[(DTid.x*2)-2]; //[DTid.x-1]
		//   last = (last/65536)-1;
	}
	else
		last = 0;

	//Build tree points
	float width = vertices[next].w;
	float3 trunkVerts[SIDES];
	float angle = (360.f / SIDES)*(3.14159f / 180);
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
	vector<float, 3> V = {vertices[next].x-vertices[last].x,vertices[next].y-vertices[last].y,vertices[next].z-vertices[last].z};
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
		float4x4 m = {	//Row 1
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
	uint outcount = (DTid.x*SIDES);
	[loop]
	for (i = 0;i<SIDES;i++){
		vers[outcount].pos.x = vertices[next].x + trunkVerts[i].x; 
		vers[outcount].pos.y = vertices[next].y + trunkVerts[i].y; 
		vers[outcount].pos.z = vertices[next].z + trunkVerts[i].z;
		vers[outcount].norm  = trunkVerts[i];
		vers[outcount].tex0 = float2(texcoord[i],next);
		outcount++;
	}
	//*/
}