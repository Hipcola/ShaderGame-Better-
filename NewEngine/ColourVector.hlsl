//--------------------------------------------------------------------------------------
// Colour
//--------------------------------------------------------------------------------------

static const float4 fvAmbient = {0.5f,0.5f,0.5f,1.0f};
static const float4 fvDiffuse = {0.5f,0.5f,0.5f,1.0f};
static const float4 fvSpecular = {1,1,1,1};
static const float fSpecularPower = 25.0f;

cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix worldMatrix;
	matrix viewMatrix;
	matrix projMatrix;
	float4 lightColour;
	float3 camPos;
	float3 lightPos;
};

struct VS_Input
{
    float4 pos  : POSITION;
};

struct PS_Input
{
    float4 pos  : SV_POSITION;
};

PS_Input VS_Main( VS_Input vertex )
{
    PS_Input vsOut = (PS_Input)0;
	vsOut.pos = vertex.pos;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );
    return vsOut;
}