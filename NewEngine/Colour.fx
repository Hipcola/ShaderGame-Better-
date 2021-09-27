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
	float4 lightColour;
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

struct VS_Input
{
    float4 pos  : POSITION;
};

struct VS_InputNorm
{
    float4 pos  : POSITION;
	float3 norm : NORMAL0;
};

struct VS_InputInstanced
{
    float4 pos  : POSITION;
	float3 instancePos : inPos;
};

struct PS_InputLighting
{
    float4 pos  : SV_POSITION;
	float3 viewDir :   TEXCOORD1;
	float3 lightDir :  TEXCOORD2;
	float3 normal :    TEXCOORD3;
};

struct PS_Input
{
    float4 pos  : SV_POSITION;
};


//float4 calcPhongLighting( float4 LColor, float3 N, float3 L, float3 V, float3 R )
//{	
//	//float4 Ia = M.Ka * ambientLight;
//	//float4 Id = M.Kd * saturate( dot(N,L) );
//	//float4 Is = M.Ks * pow( saturate(dot(R,V)), M.A );
//	float4 Ia = ambientLight;
//	float4 Id = saturate( dot(N,L) );
//	float4 Is = pow( saturate(dot(R,V)), M.A );
//	
//	return Ia + (Id + Is) * LColor;
//} 


//--------------------------------------------------------------------------------------
// VERTEX SHADER
//--------------------------------------------------------------------------------------

PS_Input VS_Main( VS_Input vertex )
{
    PS_Input vsOut = (PS_Input)0;
	vsOut.pos = vertex.pos;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );

    return vsOut;
}

PS_Input VS_MainInstanced( VS_InputInstanced vertex )
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

PS_InputLighting VS_MainLighting( VS_InputNorm vertex )
{
    PS_InputLighting vsOut = (PS_InputLighting)0;
	vsOut.pos = vertex.pos;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
	float3 fvObjectPosition = vsOut.pos;

    vsOut.pos = mul( vsOut.pos, projMatrix );

	vsOut.normal = mul( vertex.norm, worldMatrix );
	vsOut.normal = mul( vertex.norm, viewMatrix );
    vsOut.viewDir    = camPos - fvObjectPosition;
    vsOut.lightDir   = lightPos - fvObjectPosition;

    return vsOut;
}

//--------------------------------------------------------------------------------------
// PIXEL SHADER
//--------------------------------------------------------------------------------------
float4 PS_Lighting( PS_InputLighting frag ) : SV_TARGET
{ 
	//return 1.0f;      

	float3 fvLightDirection = normalize( frag.lightDir );
	float3 fvNormal         = normalize( frag.normal );
	float  fNDotL           = dot( fvNormal, fvLightDirection ); 
   
	float3 fvReflection     = normalize( ( ( 2.0f * fvNormal ) * ( fNDotL ) ) - fvLightDirection ); 
	float3 fvViewDirection  = normalize( frag.viewDir );
	float  fRDotV           = max( 0.0f, dot( fvReflection, fvViewDirection ) );
   
	float4 fvBaseColor      = float4(1.0f,1.0f,1.0f,1.0f);
   
	float4 fvTotalAmbient   = fvAmbient * fvBaseColor; 
	float4 fvTotalDiffuse   = fvDiffuse * fNDotL * fvBaseColor; 
	float4 fvTotalSpecular  = fvSpecular * pow( fRDotV, fSpecularPower );
   
	return( saturate( fvTotalAmbient + fvTotalDiffuse + fvTotalSpecular ) );
	//return txDiffuse.Sample( samLinear, input.Tex );
}

float4 PS_Main( PS_Input frag ) : SV_TARGET
{ 
	return 1.0f;      
}

technique11 FlatColourInstanced
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VS_MainInstanced() ) );
		SetGeometryShader( NULL );		
		SetPixelShader( CompileShader( ps_4_0, PS_Main() ) );
    }    
}

technique11 FlatColour
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VS_Main() ) );
		SetGeometryShader( NULL );		
		SetPixelShader( CompileShader( ps_4_0, PS_Main() ) );
    }    
}

technique11 Colour
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, VS_MainLighting() ) );
		SetGeometryShader( NULL );		
		SetPixelShader( CompileShader( ps_4_0, PS_Lighting() ) );
    }    
}