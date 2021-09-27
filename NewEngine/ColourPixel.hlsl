//--------------------------------------------------------------------------------------
// Colour
//--------------------------------------------------------------------------------------

static const float4 fvAmbient = {0.5f,0.5f,0.5f,1.0f};
static const float4 fvDiffuse = {0.5f,0.5f,0.5f,1.0f};
static const float4 fvSpecular = {1,1,1,1};
static const float fSpecularPower = 25.0f;

struct PS_Input
{
    float4 pos  : SV_POSITION;
};

float4 PS_Main( PS_Input frag ) : SV_TARGET
{ 
	return 1.0f;      
}