
Texture2D colorMap : register( t0 );
SamplerState colorSampler : register( s0 );

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
};

BlendState QuadAlphaBlendState
{
    AlphaToCoverageEnable = TRUE;
	BlendEnable[0] = TRUE;
    SrcBlend = ONE;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

struct VS_Input
{
    float3 pos  : POSITION;
    float2 tex0 : TEXCOORD0;
};


struct VS_InputInstanced
{
    float3 pos  : POSITION;
	float2 tex0 : TEXCOORD0;
	float3 instancePos : inPos;
};

struct VS_InstRot
{
    float3 pos  : POSITION;
	float2 tex0 : TEXCOORD0;
	float3 instancePos : inPos;
	float3 mat1 : inRowOne;
	float3 mat2 : inRowTwo;
	float3 mat3 : inRowThree;
	//ROTDATA
};


struct PS_Input
{
    float4 pos  : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};

float4 transform (float4 v, matrix m)
{
    float4 result;
    for ( int i = 0; i < 4; ++i )
       result[i] = v[0] * m[0][i] + v[1] * m[1][i] + v[2] + m[2][i] + v[3] * m[3][i];
    result[0] = result[0]/result[3];
    result[1] = result[1]/result[3];
    result[2] = result[2]/result[3];
    return result;
}

PS_Input VS_Main( VS_Input vertex )
{
    PS_Input vsOut = ( PS_Input )0;

	vsOut.pos.xyz = vertex.pos;
	vsOut.pos.w = 1.0f;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );
    vsOut.tex0 = vertex.tex0;

    return vsOut;
}

PS_Input VS_Instanced( VS_InputInstanced vertex )
{
    PS_Input vsOut = (PS_Input)0;

	vsOut.pos.xyz = vertex.pos;
	vsOut.pos.w = 1.0f;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
	vsOut.pos.x += vertex.instancePos.x;
	vsOut.pos.y += vertex.instancePos.y;
	vsOut.pos.z += vertex.instancePos.z;
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );
	vsOut.tex0 = vertex.tex0;

    return vsOut;
}


PS_Input VS_InstancedRot( VS_InstRot vertex )
{
    PS_Input vsOut = (PS_Input)0;

	vsOut.pos.xyz = vertex.pos;
	vsOut.pos.w = 1.0f;
    //vsOut.pos = mul( vsOut.pos, worldMatrix );
	matrix m = { 
				vertex.mat1.x, vertex.mat1.y, vertex.mat1.z, 0.0f,
				vertex.mat2.x, vertex.mat2.y, vertex.mat2.z, 0.0f,
				vertex.mat3.x, vertex.mat3.y, vertex.mat3.z, 0.0f,
				0.0f,0.0f,0.0f,1.0f
			};
	vsOut.pos = mul( vsOut.pos, m); 
	//vsOut.pos = transform( vsOut.pos, m );
	vsOut.pos.x += vertex.instancePos.x;
	vsOut.pos.y += vertex.instancePos.y;
	vsOut.pos.z += vertex.instancePos.z;
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );
	vsOut.tex0 = vertex.tex0;

    return vsOut;
}