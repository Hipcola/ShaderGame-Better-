
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
};

cbuffer cbNeverChanges : register( b1 )
{
    matrix viewMatrix;
};

cbuffer cbChangeOnResize : register( b2 )
{
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

struct PS_Input
{
    float4 pos  : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};

float4 PS_Main( PS_Input frag ) : SV_TARGET
{
	float4 col;
	col = colorMap.Sample( colorSampler, frag.tex0); //colorSampler samLinear
	return col;//float4( frag.tex0.x, 0.0f, frag.tex0.y, 0.0f );
}