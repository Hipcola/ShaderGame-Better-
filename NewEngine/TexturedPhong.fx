//--------------------------------------------------------------------------------------
// Colour
//--------------------------------------------------------------------------------------

static const float4 fvAmbient = {0.5f,0.5f,0.5f,1.0f};
static const float4 fvDiffuse = {0.5f,0.5f,0.5f,1.0f};
static const float4 fvSpecular = {1,1,1,1};
static const float fSpecularPower = 25.0f;

SamplerState sampler_ : register( s0 );
Texture2D map : register( t0 );

cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix worldMatrix;
	float3 camPos;
	float3 lightPos;
};

cbuffer cbNeverChanges : register( b1 )
{
    matrix viewMatrix;
};

cbuffer cbChangeOnResize : register( b2 )
{
    matrix projMatrix;
};

struct VSid
{
	//float4  color   : COLOR;
	uint id : SV_VERTEXID;
};

struct VS_Input
{
    float3 pos  : POSITION;
    float3 norm : NORMAL0;
	float2 tex  : TEXCOORD0;
};

StructuredBuffer<VS_Input> gVertexBuffer;

struct VS_InputInstanced
{
    float4 pos  : POSITION;
	float3 instancePos : inPos;
};

struct PS_Input
{
    float4 pos  : SV_POSITION;
	float3 viewDir :   TEXCOORD1;
	float3 lightDir :  TEXCOORD2;
	float3 normal :    TEXCOORD3;
	float2 tex  :      TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// VERTEX SHADER
//--------------------------------------------------------------------------------------

PS_Input VS_Main( VS_Input vertex )
{
    PS_Input vsOut = (PS_Input)0;
	vsOut.pos.xyz = vertex.pos;
	vsOut.pos.w = 1.0f;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
	float3 fvObjectPosition = vsOut.pos;

    vsOut.pos = mul( vsOut.pos, projMatrix );

	vsOut.normal = mul( vertex.norm, worldMatrix );
	vsOut.normal = mul( vertex.norm, viewMatrix );
    vsOut.viewDir    = camPos - fvObjectPosition;
    vsOut.lightDir   = lightPos - fvObjectPosition;
	vsOut.tex = vertex.tex;

    return vsOut;
}

PS_Input VS_Buff( VSid input )
{
	PS_Input vsOut = (PS_Input)0;	
	vsOut.pos.xyz = gVertexBuffer[input.id].pos;
	vsOut.pos.w = 1.0f;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
	float3 fvObjectPosition = vsOut.pos;
    vsOut.pos = mul( vsOut.pos, projMatrix );
	vsOut.normal = mul( gVertexBuffer[input.id].norm, worldMatrix );
	vsOut.normal = mul( gVertexBuffer[input.id].norm, viewMatrix );
    vsOut.viewDir    = camPos - fvObjectPosition;
    vsOut.lightDir   = lightPos - fvObjectPosition;
	//vsOut.tex = gVertexBuffer[input.id].tex;
	float2 tex = gVertexBuffer[input.id].tex;
	vsOut.tex = tex;
	return vsOut;
}

PS_Input VS_Instanced( VS_InputInstanced vertex )
{
    PS_Input vsOut = (PS_Input)0;
	vsOut.pos = vertex.pos;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
	vsOut.pos.x += vertex.instancePos.x;
	vsOut.pos.y += vertex.instancePos.y;
	vsOut.pos.z += vertex.instancePos.z;
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );

    return vsOut;
}

//--------------------------------------------------------------------------------------
// PIXEL SHADER
//--------------------------------------------------------------------------------------
float4 PS_Main( PS_Input frag ) : SV_TARGET
{ 
	//return 1.0f;      

	/*
	float3 fvLightDirection = normalize( frag.lightDir );
	float3 fvNormal         = normalize( frag.normal );
	float  fNDotL           = dot( fvNormal, fvLightDirection ); 
   
	float3 fvReflection     = normalize( ( ( 2.0f * fvNormal ) * ( fNDotL ) ) - fvLightDirection ); 
	float3 fvViewDirection  = normalize( frag.viewDir );
	float  fRDotV           = max( 0.0f, dot( fvReflection, fvViewDirection ) );
   
	float4 fvBaseColor      = map.Sample( sampler_, frag.tex); //float4(1.0f,1.0f,1.0f,1.0f);
   
	float4 fvTotalAmbient   = fvAmbient * fvBaseColor; 
	float4 fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
	float4 fvTotalSpecular  = fvSpecular * pow( fRDotV, fSpecularPower );
   
	return ( saturate( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular ) );
	//*/
	return 0;
	//return txDiffuse.Sample( samLinear, input.Tex );
}

technique11 TexturePhong
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VS_Main() ) );
		SetGeometryShader( NULL );		
		SetPixelShader( CompileShader( ps_4_0, PS_Main() ) );
    }    
}

technique11 TexturePhongBuffer
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VS_Buff() ) );
		SetGeometryShader( NULL );		
		SetPixelShader( CompileShader( ps_4_0, PS_Main() ) );
    }    
}

technique11 TexturePhongInstanced
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VS_Instanced() ) );
		SetGeometryShader( NULL );		
		SetPixelShader( CompileShader( ps_4_0, PS_Main() ) );
    }    
}