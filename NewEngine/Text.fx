/*
      Text Shader built from: http://www.rastertek.com/dx11tut13.html
				
*/


Texture2D shaderTexture : register( t0 );
SamplerState colorSampler : register( s0 );

BlendState QuadAlphaBlendState
{
    AlphaToCoverageEnable = TRUE;
	BlendEnable[0] = TRUE;
    SrcBlend = ONE;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix worldMatrix;
};

cbuffer cbNeverChanges : register( b1 )
{
    matrix viewMatrix;
    float4 pixelColor;
};

cbuffer cbChangeOnResize : register( b2 )
{
    matrix projMatrix;
};

struct VS_Input
{
    float4 pos  : POSITION;
    float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
    float4 pos  : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};

PS_Input VS_Main( VS_Input vertex )
{
    PS_Input vsOut = ( PS_Input )0;

    vsOut.pos = vertex.pos;
    vsOut.pos = mul( vsOut.pos, worldMatrix );
    vsOut.pos = mul( vsOut.pos, viewMatrix );
    vsOut.pos = mul( vsOut.pos, projMatrix );
    vsOut.tex0 = vertex.tex0;

    return vsOut;
}


float4 PS_Main( PS_Input frag ) : SV_TARGET
{
    float4 color;	
	
    // Sample the texture pixel at this location.
    color = shaderTexture.Sample(colorSampler, frag.tex0);
	
    // If the color is black on the texture then treat this pixel as transparent.
    if(color.r == 0.0f)
    {
	color.a = 0.0f;
    }
	
    // If the color is other than black on the texture then this is a pixel in the font so draw it using the font pixel color.
    else
    {
	color.a = 1.0f;
	color = color * pixelColor;
    }

    return color;
}


technique11 Text
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Main() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Main() ) );
		SetBlendState( QuadAlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
    }
}
